#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define SERVER_IP "3.226.148.224"

int main() {
    int client_fd;
    struct sockaddr_in server_addr;

    // Create socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("invalid address");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connection failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // Send authentication message
    const char *auth_message = "user|pass";
    if (write(client_fd, auth_message, strlen(auth_message)) < 0) {
        perror("write failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // Read server response
    char buffer[1024] = {0};
    int read_len = read(client_fd, buffer, sizeof(buffer) - 1);
    if (read_len < 0) {
        perror("read failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // Process server response
    if (read_len == 0) {
        printf("Server closed connection without responding\n");
    } else if (strcmp(buffer, "AUTH_SUCCESS") == 0) {
        printf("Authentication successful\n");
    } else if (strcmp(buffer, "AUTH_FAILURE") == 0) {
        printf("Authentication failed\n");
    } else {
        printf("Unknown response: %s\n", buffer);
    }

    // Close connection
    close(client_fd);
    return 0;
}