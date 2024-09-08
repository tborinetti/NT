#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_SERVER_PORT "4040"

void server_connected(SOCKET server_socket);


int __cdecl main(int argc, char **argv)
{
    WSADATA wsaData;
    SOCKET ServerConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
        *ptr = NULL,
        hints;
    int iResult;

    // Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_SERVER_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL;ptr = ptr->ai_next) 
    {

        // Create a SOCKET for connecting to server
        ServerConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ServerConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ServerConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ServerConnectSocket);
            ServerConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ServerConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    server_connected(ServerConnectSocket);

    return 0;
}


void server_connected(SOCKET server_socket) 
{
    SOCKET *server = &server_socket;
    int result;

    char sendbuf[DEFAULT_BUFLEN];
    int sendbuflen = DEFAULT_BUFLEN;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    char addrbuf[DEFAULT_BUFLEN];
    int addrbuflen = DEFAULT_BUFLEN;

    int sendresult;
    char *joinmsg = "User connected\n";

    struct sockaddr name;
    int namelen = (int)sizeof(name);

    getpeername(server_socket, &name, &namelen);
    WSAAddressToStringW(&name, namelen, NULL, &sendbuf, sendbuflen);

    snprintf(sendbuf, sendbuflen, "Connected with address: %s\n", addrbuf);
    
    // Send an initial buffer
    result = send(*server, joinmsg, (int)strlen(joinmsg), 0);
    if (result == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(*server);
        WSACleanup();
        return;
    }
    printf("Bytes Sent: %ld\n", result);

    do {

        result = recv(*server, recvbuf, recvbuflen, 0);
        if (result > 0)
        {
            printf("Bytes received: %d\n", result);
            printf("Data: %s", recvbuf);
            sendresult = send(*server, sendbuf, sendbuflen, 0);
            if (sendresult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(*server);
                WSACleanup();
                return;
            }
        }

        else if (result == 0)
            printf("Waiting for data\n");
        else
            printf("Disconnected from server: %d\n", WSAGetLastError());

    } while (result >= 0);

    // shutdown the connection since no more data will be sent
    result = shutdown(*server, SD_SEND);
    if (result == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(*server);
        WSACleanup();
        return;
    }
    // cleanup
    closesocket(*server);
    WSACleanup();
}
