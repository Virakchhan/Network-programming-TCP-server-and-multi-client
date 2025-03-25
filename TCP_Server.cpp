#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 8000
#define BUFFER_SIZE 512
#define MAX_CLIENTS 10

typedef struct {
    SOCKET socket;
    int id;
    char name[20];  // Store client name
} ClientInfo;

DWORD WINAPI HandleClient(LPVOID param) {
    ClientInfo* client = (ClientInfo*)param;
    SOCKET clientSocket = client->socket;
    char buffer[BUFFER_SIZE];
    int bytesReceived;

    // Receive client name
    bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesReceived <= 0) {
        printf("[Client %d] Connection closed or error.\n", client->id);
        closesocket(clientSocket);
        free(client);
        return 0;
    }

    buffer[bytesReceived] = '\0';
    strncpy(client->name, buffer, sizeof(client->name) - 1);
    client->name[sizeof(client->name) - 1] = '\0';  // Ensure null-termination

    printf("[%s] connected\n", client->name);  // Print client name

    // Communication loop
    while ((bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytesReceived] = '\0';
        printf("[%s] Sent: %s\n", client->name, buffer);  // Print message with client name

        // Send response
        send(clientSocket, buffer, bytesReceived, 0);
    }

    printf("[%s] Disconnected.\n", client->name);  // Print when client disconnects
    closesocket(clientSocket);
    free(client);
    return 0;
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    int clientId = 0;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed!\n");
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("Socket creation failed!\n");
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Bind failed!\n");
        return 1;
    }

    if (listen(serverSocket, MAX_CLIENTS) == SOCKET_ERROR) {
        printf("Listen failed!\n");
        return 1;
    }

    printf("[Server] Listening on port %d...\n", SERVER_PORT);

    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            printf("Accept failed!\n");
            continue;
        }

        ClientInfo* client = (ClientInfo*)malloc(sizeof(ClientInfo));
        client->socket = clientSocket;
        client->id = ++clientId;

        printf("[Server] New client connected: %d\n", client->id);
        CreateThread(NULL, 0, HandleClient, (LPVOID)client, 0, NULL);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
