#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 8080
#define BACKLOG 5

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Step 1: Log socket creation and configuration
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully: fd=%d\n", server_fd);

    int flags = fcntl(server_fd, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl F_GETFL failed");
    } else {
        printf("Socket flags: %d (O_NONBLOCK: %s)\n", flags, (flags & O_NONBLOCK) ? "yes" : "no");
    }

    // Step 2: Log binding and listening
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Socket bound to 0.0.0.0:%d\n", PORT);

    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server listening with backlog=%d\n", BACKLOG);

    // Step 3 & 4: Accept loop with detailed logging
    while (1) {
        printf("Waiting for connection...\n");
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            fprintf(stderr, "accept failed: %s (errno=%d)\n", strerror(errno), errno);
            // Log socket state after failure
            flags = fcntl(server_fd, F_GETFL, 0);
            if (flags < 0) {
                perror("fcntl F_GETFL failed after accept error");
            } else {
                printf("Socket flags after error: %d (O_NONBLOCK: %s)\n", 
                       flags, (flags & O_NONBLOCK) ? "yes" : "no");
            }
            continue;
        }
        printf("Connection accepted: client_fd=%d\n", client_fd);

        // Step 5: Log client handling
        char buffer[1024] = {0};
        int read_len = read(client_fd, buffer, sizeof(buffer) - 1);
        if (read_len < 0) {
            perror("read failed");
            close(client_fd);
            continue;
        }
        printf("Received from client: %s\n", buffer);

        // Simple authentication (username|password)
        char *username = strtok(buffer, "|");
        char *password = strtok(NULL, "|");
        const char *valid_username = "user";
        const char *valid_password = "pass";

        if (username && password && 
            strcmp(username, valid_username) == 0 && 
            strcmp(password, valid_password) == 0) {
            printf("Authentication successful\n");
            write(client_fd, "AUTH_SUCCESS", strlen("AUTH_SUCCESS"));
        } else {
            printf("Authentication failed\n");
            write(client_fd, "AUTH_FAILURE", strlen("AUTH_FAILURE"));
        }
        close(client_fd);
    }

    close(server_fd);
    return 0;
}