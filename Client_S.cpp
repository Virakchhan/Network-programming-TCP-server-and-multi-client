#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVERPORT 8000
#define BUFSIZE 512

char *SERVERIP = "127.0.0.1";

void err_quit(const char *msg) {
    fprintf(stderr, "%s failed: %d\n", msg, WSAGetLastError());
    exit(1);
}

void err_display(const char *msg) {
    fprintf(stderr, "%s error: %d\n", msg, WSAGetLastError());
}

int main() {
    int retval;
    char client_type[] = "Client_S"; // Name for this client
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in serveraddr;
    char buf[BUFSIZE + 1];
    int len;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup failed!\n");
        return 1;
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // Set up server address
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);

    // Connect to the server
    retval = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

    // Send client type (e.g., "Client_S")
    retval = send(sock, client_type, (int)strlen(client_type), 0);
    if (retval == SOCKET_ERROR) err_quit("send()");

    // Communication loop
    while (1) {
        // Get user input
        printf("\n[Send Message] ");
        if (fgets(buf, BUFSIZE, stdin) == NULL)
            break;

        // Remove trailing newline
        len = (int)strlen(buf);
        if (len > 0 && buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)
            break;

        // Send message
        retval = send(sock, buf, (int)strlen(buf), 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }
        printf("[TCP Client] Sent %d bytes.\n", retval);

        // Receive response
        retval = recv(sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        } else if (retval == 0)
            break;

        // Null-terminate received data and print it
        buf[retval] = '\0';
        printf("[TCP Client] Received %d bytes.\n", retval);
        printf("[Server Response] %s\n", buf);
    }

    // Close socket
    closesocket(sock);

    // Cleanup Winsock
    WSACleanup();

    // Pause before exiting
    printf("Press Enter to exit...");
    getchar();

    return 0;
}
