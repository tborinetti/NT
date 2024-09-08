#define WIN32_LEAN_AND_MEAN  

#include <windows.h>        
#include <winsock2.h> 
#include <Ws2ipdef.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <synchapi.h>


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "4040"
#define MAX_THREADS 1

DWORD WINAPI user_connected(LPVOID lpParam);
void print_sock_addr(SOCKET s, struct sockaddr addr, int addrlen);

int main(void)
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
        struct sockaddr addr;
        int addrlen = (int)sizeof(addr);

        // Accept a client socket
        ClientSocket = accept(ListenSocket, &addr, &addrlen);
        print_sock_addr(ClientSocket, &addr, &addrlen);
        // If client socket already connected dont create a new thread

        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        HANDLE new_thread = CreateThread(NULL, 0, user_connected, ClientSocket, 0, NULL);
        if (new_thread == INVALID_HANDLE_VALUE) {
            printf("Error creating thread.\n");
        }
        else {
            printf("Thread created.\n");
        }
        
    }


    // No longer need server socket
    // cleanup
    closesocket(ListenSocket);
    
    WSACleanup();

    return 0;

}


DWORD WINAPI user_connected(LPVOID lpParam)
{
    
    SOCKET *client = (SOCKET *)lpParam;
    int result;

    char sendbuf[DEFAULT_BUFLEN];
    int sendbuflen = DEFAULT_BUFLEN;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    char addrbuf[DEFAULT_BUFLEN];
    int addrbuflen = DEFAULT_BUFLEN;

    int sendresult;
    char *connectack = "Thanks for connecting";

    struct sockaddr name;
    int namelen = (int)sizeof(name);

    result = getpeername(*client, &name, &namelen);
    if (result == SOCKET_ERROR)
    {
        printf("peername failed with error: %d\n", WSAGetLastError());
        closesocket(*client);
        WSACleanup();
        return 1;
    }

    result = WSAAddressToStringW(&name, namelen, NULL, &addrbuf, addrbuflen);
    if (result == SOCKET_ERROR)
    {
        printf("string formatting failed with error: %d\n", WSAGetLastError());
        closesocket(*client);
        WSACleanup();
        return 1;
    }

    result = snprintf(sendbuf, sendbuflen, "Connected with address: %s\n", addrbuf);
    if (result < 0) printf("error printing string");

    // Receive until the peer shuts down the connection
    do {

        result = recv(*client, recvbuf, recvbuflen, 0);
        if (result > 0) {
            printf("Bytes received: %d\n", result);

            // Echo the buffer back to the sender
            sendresult = send(*client, recvbuf, result, 0);
            if (sendresult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(*client);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", sendresult);
        }
        else if (result == 0)
            //printf("Waiting for data\n");
            ;
        else {
            printf("User disconnected: %d\n", WSAGetLastError());
            closesocket(*client);
            WSACleanup();
            return 1;
        }

    } while (result >= 0);

    // shutdown the connection since we're done
    result = shutdown(*client, SD_SEND);
    if (result == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(*client);
        WSACleanup();
        return 1; 
    }
}


void print_sock_addr(SOCKET s, struct sockaddr addr, int addrlen) {
    struct sockaddr name;
    int namelen = (int)sizeof(name);
    char addrbuf[DEFAULT_BUFLEN];
    int addrbuflen = DEFAULT_BUFLEN;
    int result;

    result = getpeername(s, &name, &namelen);
    if (result == SOCKET_ERROR)
    {
        printf("peername failed with error: %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return 1;
    }

    result = WSAAddressToStringW(&name, namelen, NULL, &addrbuf, addrbuflen);
    if (result == SOCKET_ERROR)
    {
        printf("string formatting failed with error: %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return 1;
    }
    printf("Address is: %s", addrbuf);
}