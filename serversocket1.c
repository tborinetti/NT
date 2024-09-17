#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "4040"
#define MAX_USERS 5


















int __cdecl main(void)
{
    WSADATA wsaData;
    SOCKET ListenSocket = INVALID_SOCKET;
    int iResult;
    struct addrinfo *result = NULL;
    struct addrinfo hints;

    SOCKET ClientSocket = INVALID_SOCKET;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    char sendbuf[DEFAULT_BUFLEN];
    int sendbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }



    // Accept a client socket
    struct sockaddr_in client_addr = { 0 };
    struct sockaddr_in* clientp = &client_addr;
    int addr_len = sizeof(client_addr);
    char addrbuf[DEFAULT_BUFLEN];
    int addrbuflen = DEFAULT_BUFLEN;

    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    int testresult = 0;

    testresult = getpeername(ClientSocket, &client_addr, &addr_len);
    if (testresult == SOCKET_ERROR)
    {
        printf("peername failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    const struct in_addr* clientin_addr = &client_addr.sin_addr;
    addr_len = sizeof(addrbuf);
    testresult = inet_ntop(AF_INET, (const void*)clientin_addr, addrbuf, addr_len);

    if (testresult == SOCKET_ERROR)
    {
        printf("string formatting failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    testresult = snprintf(sendbuf, sendbuflen, "You are connected with address: %s", addrbuf);
    if (testresult <= 0)
    {
        printf("snprintf formatting failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    // Receive until the peer shuts down the connection
    do {

        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            recvbuf[iResult] = '\0';
            printf("Bytes received: %d\n", iResult);
            printf("Data: %s\n", recvbuf);
            // Echo the buffer back to the sender
            iSendResult = send(ClientSocket, sendbuf, sendbuflen, 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
        }
        else if (iResult == 0)
            printf("Waiting...\n");
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult >= 0);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }



    // cleanup
    closesocket(ClientSocket);
    // No longer need server socket
    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}