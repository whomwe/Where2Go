//Server Socekt
#include "HTTPSServer.h"
#include <fstream>

using namespace std;

//initialise SSL security layer as "handleClient"
void handleClient(SSL* ssl) {
	char buffer[200];

	while (true) {
		ZeroMemory(buffer, sizeof(buffer));
		int bytesReceived = SSL_read(ssl, buffer, sizeof(buffer));
		if (bytesReceived <= 0) break;

		cout << "[Thread] Client says: " << buffer << endl;
		SSL_write(ssl, buffer, strlen(buffer));
	}

	SSL_shutdown(ssl);
	SSL_free(ssl);
	cout << "[Thread] Client disconnected." << endl;
}

int main()
{
	//step 1initialise WSA by invoking the DLL's 
	SOCKET serverSocket, acceptSocket;
	int port = 27015;
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	int wsaerr = WSAStartup(wVersionRequested, &wsaData);
	if (wsaerr != 0) {
		cout << "Winsock dll not found" << endl;
		return 0;
	}
	else {
		cout << "Winsock dll found!" << endl;
		cout << "status: " << wsaData.szSystemStatus << endl;
	}

	//step 2a - create the server socket AND CLIENT SOCKET 
	//serverSocket = INVALID_SOCKET;
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) {
		cout << "Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return 0;
	}
	else {
		cout << "socket() is OK!" << endl;
	}



	//step 3 - bind the socket to the relevant IP and Port number (using the bind function. it takes 3 parameters: the socket, the address of a socket variable, and the socket length.)
	sockaddr_in service;
	//AF_INET for family used as its TCP connection
	service.sin_family = AF_INET;
	InetPton(AF_INET, "127.0.0.1", &service.sin_addr.s_addr);
	service.sin_port = htons(port);

	//error handling for port binding 
	if (bind(serverSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
		cout << "bind() failed: " << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return 0;
	}
	else {
		cout << "bind() is okay" << endl;
	}	

	//step 4a - listen out for connections 
	if (listen(serverSocket, 1) == SOCKET_ERROR) {
		cout << "listen(): Error listening on socket" << WSAGetLastError() << endl;
		return 0;
	}
	else {
		cout << "listen() OK, awaiting connections for port "<< port << "... " << endl;
	}

	//Initialise OpenSSL
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	const SSL_METHOD* method = TLS_server_method();
	SSL_CTX* ctx = SSL_CTX_new(method);

	if (!ctx) {
		ERR_print_errors_fp(stderr);
		return 1;
	}

	ifstream certTest("certs/server.crt");
	if (!certTest.is_open()) {
		cerr << "[ERROR] server.crt not found!" << endl;
	}

	if (SSL_CTX_use_certificate_file(ctx, "certs/server.crt", SSL_FILETYPE_PEM) <= 0 ||
		SSL_CTX_use_PrivateKey_file(ctx, "certs/server.key", SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		return 1;
	}

	//step 4c - accept connections that come in : pauses execution until client establishes connection with socket, which we then accept and move on 
	acceptSocket = accept(serverSocket, NULL, NULL);
	if (acceptSocket == INVALID_SOCKET) {
		cout << "accept() failed: " << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}
	cout << "connection accepted" << endl;

	//step 4d - Wrap the accepted socket in secure stream
	SSL* ssl = SSL_new(ctx);
	SSL_set_fd(ssl, acceptSocket);

	if (SSL_accept(ssl) <= 0) {
		ERR_print_errors_fp(stderr);
	}
	else {
		handleClient(ssl);

	}

	// Clean up (AFTER accept runs) if message is "shutdown"
		closesocket(acceptSocket);
		closesocket(serverSocket);
		WSACleanup();
		return 0;
	

}
