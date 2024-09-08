#define WIN32_LEAN_AND_MEAN  

#include <windows.h>        
#include <winsock2.h> 
#include <Ws2ipdef.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <synchapi.h>


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "4040"
#define MAX_THREADS "5"

void user_connected();

void main(void)
{
    WSADATA wsaData;
    int iResult;
    int thread_size;



    SOCKET ListenSocket = INVALID_SOCKET;


    struct addrinfo *result = NULL;
    struct addrinfo hints;




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
    while (1)
    {
        SOCKET ClientSocket = INVALID_SOCKET;
        // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);

        // If client socket already connected dont create a new thread

        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        CreateThread(NULL, 0, user_connected, &ClientSocket, 0, NULL);
        
        closesocket(ClientSocket);
    }


    // No longer need server socket
    // cleanup
    closesocket(ListenSocket);
    
    WSACleanup();

    return 0;

}


void user_connected(SOCKET *client_socket)
{
    int result;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int iSendResult;

    // Receive until the peer shuts down the connection
    do {

        result = recv(client_socket, recvbuf, recvbuflen, 0);
        if (result > 0) {
            printf("Bytes received: %d\n", result);

            // Echo the buffer back to the sender
            iSendResult = send(client_socket, recvbuf, result, 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(client_socket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
        }
        else if (result == 0)
            printf("Waiting for data\n");
        else {
            printf("User disconnected: %d\n", WSAGetLastError());
            closesocket(client_socket);
            WSACleanup();
            return 1;
        }

    } while (result >= 0);

    // shutdown the connection since we're done
    result = shutdown(client_socket, SD_SEND);
    if (result == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }
}