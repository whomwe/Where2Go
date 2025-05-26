#include "HTTPSServer.h"
#include "HTTPSServer.cpp"

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

	//step 2 - create the server socket AND CLIENT SOCKET 
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

	// Create the clientSocket
	SOCKET clientSocket = INVALID_SOCKET;
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET) {
		cout << "Error at clientSocket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return 0;
	}
	else {
		cout << "clientSocket() is OK!" << endl;
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

	if (SSL_CTX_use_certificate_file(ctx, "certs/server.crt", SSL_FILETYPE_PEM) <= 0 ||
		SSL_CTX_use_PrivateKey_file(ctx, "certs/server.key", SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		return 1;
	}
	//CLIENT SOCKET CONFIGURATION

	//connect to the server socket
	sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	InetPton(AF_INET, "127.0.0.1", &clientService.sin_addr.s_addr);
	clientService.sin_port = htons(port);
	if (connect(clientSocket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		cout << "Client: connect() - failed to connect" << endl;
		WSACleanup();
		return 0;
	}
	else {
		cout << "Client: Connect() is OK, you may now send & recieve data" << endl;
	}
	system("pause");

	//step 4b - accept connections that come in : pauses execution until client establishes connection with socket, which we then accept and move on 
	acceptSocket = accept(serverSocket, NULL, NULL);
	if (acceptSocket == INVALID_SOCKET) {
		cout << "accept() failed: " << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}
	cout << "connection accepted" << endl;

	//step 4c - Wrap the accepted socket in secure stream
	cout << "connection accepted" << endl;
	SSL* ssl = SSL_new(ctx);
	SSL_set_fd(ssl, acceptSocket);

	if (SSL_accept(ssl) <= 0) {
		ERR_print_errors_fp(stderr);
	}
	else {
		SSL_write(ssl, "Hello over HTTPS!", 18);

	}
	//SEND AND RECIEVE SERVER


	//SENDING DATA VIA TCP 
	char buffer[200];

	while (true) {
		cout << "enter the message you would like to send to the server (to exit, type 'exit') : ";
		cin.getline(buffer, 200);

		//go to accept when exit is typed
		if (strcmp(buffer, "exit") == 0) break;

		int bytecount = send(clientSocket, buffer, 200, 0);

		if (bytecount > 0) {
			cout << "message sent: " << buffer << endl;

		}
		else {
			WSACleanup();
		}

 		//chat to the client, recieve data and clear buffer to recieve new message synchoronusly and send & recieve mulitple times
		ZeroMemory(buffer, 200);

		int bytecount2 = recv(acceptSocket, buffer, 200, 0);

		if (bytecount2 > 0) {
			cout << "message recieved! you said: " << buffer << endl;
		}
		else {
			WSACleanup();
		}
	}

	// Clean up (AFTER accept runs) if message is "shutdown"
		if (buffer == "shutdown") {
			closesocket(clientSocket);
			closesocket(acceptSocket);
			closesocket(serverSocket);
			WSACleanup();
			return 0;
		}

}
