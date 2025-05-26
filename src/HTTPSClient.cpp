//CLIENT SOCKET CREATED IN A NEW FILE
#include "HTTPSServer.h"
#include "HTTPSServer.cpp"

using namespace std;

int main() {
    WSAData wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    const SSL_METHOD* method = TLS_client_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    InetPton(AF_INET, "127.0.0.1", &serverAddr.sin_addr.s_addr);
    serverAddr.sin_port = htons(27015);

    if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "Connect failed: " << WSAGetLastError() << endl;
        return 1;
    }

    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);

    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    char msg[200];
    while (true) {
        cout << "[Client] Enter message: ";
        cin.getline(msg, 200);

        if (strcmp(msg, "exit") == 0) break;

        SSL_write(ssl, msg, strlen(msg));

        char response[200] = {};
        SSL_read(ssl, response, sizeof(response));
        cout << "[Client] Server says: " << response << endl;
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    closesocket(sock);
    SSL_CTX_free(ctx);
    WSACleanup();
    return 0;
}