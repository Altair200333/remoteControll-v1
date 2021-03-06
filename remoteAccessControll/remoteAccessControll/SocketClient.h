#pragma once
#include <iostream>
#include <string>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

class SocketClient final
{
	std::string DEFAULT_PORt = "27015";
	
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	std::vector<char> recvbuf;
	int iResult;
	int recvbuflen = 5000000;

public:
	std::string port;
	std::string address;
	void init()
	{
		port = DEFAULT_PORt;
		address = "-1";
	}
	bool remConnect()
	{
		if (address == "-1")
			address = "localhost";

		recvbuf = std::vector<char>(recvbuflen,0);
		
		if (address.empty()) 
		{
			std::cout << "usage: <appname> server-name\n";
			return false;
		}

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) 
		{
			printf("WSAStartup failed with error: %d\n", iResult);
			return false;
		}

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Resolve the server address and port
		iResult = getaddrinfo(address.c_str(), port.c_str(), &hints, &result);
		if (iResult != 0) {
			printf("getaddrinfo failed with error: %d\n", iResult);
			WSACleanup();
			return false;
		}

		// Attempt to connect to an address until one succeeds
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
		{
			// Create a SOCKET for connecting to server
			ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
				ptr->ai_protocol);
			if (ConnectSocket == INVALID_SOCKET) {
				printf("socket failed with error: %ld\n", WSAGetLastError());
				WSACleanup();
				return false;
			}

			// Connect to server.
			iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				closesocket(ConnectSocket);
				ConnectSocket = INVALID_SOCKET;
				continue;
			}
			break;
		}
		freeaddrinfo(result);

		if (ConnectSocket == INVALID_SOCKET) {
			printf("Unable to connect to server!\n");
			WSACleanup();
			return false;
		}
		
		return true;
	}
	std::vector<char> sendData(const std::string& sendData)
	{
		// Send an initial buffer
		iResult = send(ConnectSocket, sendData.c_str(), sendData.length(), 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return {};
		}

		printf("Bytes Sent: %ld\n", iResult);
		receiveResponse();
		auto result = std::vector<char>(recvbuf.begin(), recvbuf.end());
		recvbuf = std::vector<char>(recvbuflen, '\0');
		return result;
	}
	std::vector<char> sendData(const std::vector<char>& sendData)
	{
		// Send an initial buffer
		iResult = send(ConnectSocket, sendData.data(), sendData.size(), 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return {};
		}

		printf("Bytes Sent: %ld\n", iResult);
		receiveResponse();
		auto result = std::vector<char>(recvbuf.begin(), recvbuf.end());
		recvbuf = std::vector<char>(recvbuflen, '\0');
		return result;
	}
	bool closeConnection()
	{
		// shutdown the connection since no more data will be sent
		iResult = shutdown(ConnectSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			printf("shutdown failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return false;
		}

		//receiveResponse();

		// cleanup
		closesocket(ConnectSocket);
		WSACleanup();
		
		return true;
	}
	
	void receiveResponse()
	{
		// Receive until the peer closes the connection
		
		iResult = recv(ConnectSocket, &recvbuf[0], recvbuf.size()*sizeof(char), 0);
		if (iResult > 0)
			printf("Bytes received: %d\n", iResult);
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());
	}
};
