#include <stdio.h>
#include <winsock2.h>
#include <stdbool.h>

#define SERVER_IP "192.168.1.229" // Change this to the IP address of your server
#define SERVER_PORT 4242

int main()
{
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_addr;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("WSAStartup failed\n");
        return 1;
    }

    // Create a socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    // Set up the server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &(server_addr.sin_addr)) <= 0)
    {
        printf("Invalid address/Address not supported\n");
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("Connection failed\n");

        // Print the error code
        int error_code = WSAGetLastError();
        printf("Error code: %d\n", error_code);

        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    // While Loop
    while (true)
    {
        // Send a message to the server
        char message[256];
        printf("Enter a message to send to the server: ");
        fgets(message, 256, stdin);
        send(client_socket, message, strlen(message), 0);
    }

    // Close the socket and cleanup Winsock
    closesocket(client_socket);
    WSACleanup();

    return 0;
}
