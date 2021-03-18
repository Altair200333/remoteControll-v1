#pragma once
#include <functional>
#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


class SocketServer final
{
    std::string DEFAULT_PORt = "27015";

    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    std::vector<char> recvbuf;
    int recvbuflen = 5000000;
public:
    std::string port = DEFAULT_PORt;

	void init()
	{
        recvbuf = std::vector<char>(recvbuflen, 0);
	}
	bool remListen()
	{
        // Initialize Winsock
        iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            printf("WSAStartup failed with error: %d\n", iResult);
            return false;
        }

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        // Resolve the server address and port
        iResult = getaddrinfo(NULL, port.c_str(), &hints, &result);
        if (iResult != 0) {
            printf("getaddrinfo failed with error: %d\n", iResult);
            WSACleanup();
            return false;
        }

        // Create a SOCKET for connecting to server
        ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (ListenSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            freeaddrinfo(result);
            WSACleanup();
            return false;
        }

        // Setup the TCP listening socket
        iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            printf("bind failed with error: %d\n", WSAGetLastError());
            freeaddrinfo(result);
            closesocket(ListenSocket);
            WSACleanup();
            return false;
        }

        freeaddrinfo(result);

        iResult = listen(ListenSocket, SOMAXCONN);
        if (iResult == SOCKET_ERROR) {
            printf("listen failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return false;
        }

        return true;
	}
	bool acceptClient()
	{
        // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return false;
        }
        return true;
	}
	bool closeSocket()
	{
        closesocket(ListenSocket);
        return true;
	}
	bool listenLoop(std::function<std::pair<bool, std::vector<char>>(std::vector<char>)> onDataReceived)
	{
        // Receive until the peer shuts down the connection
        do {

            iResult = recv(ClientSocket, &recvbuf[0], recvbuflen, 0);
            if (iResult > 0)
            {
                auto res = onDataReceived(recvbuf);
            	if(!res.first)
                    return false;
                printf("Bytes received: %d\n", iResult);

                // Echo the buffer back to the sender
                iSendResult = send(ClientSocket, res.second.data(), res.second.size(), 0);
                if (iSendResult == SOCKET_ERROR) 
                {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(ClientSocket);
                    WSACleanup();
                    return false;
                }
                printf("Bytes sent: %d\n", iSendResult);
                recvbuf = std::vector<char>(recvbuflen, '\0');
            }
            else if (iResult == 0)
                printf("Connection closing...\n");
            else {
                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return false;
            }
        } while (iResult > 0);
	}
};
