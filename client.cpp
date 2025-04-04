#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>  // Contains AF_INET, sockaddr_in, etc.
#include <arpa/inet.h>   // inet_pton()
#include <unistd.h>
#include <iostream>
#include <cstring>       // memset(), strerror()
#include <cerrno>        // errno

#define PORT 8080

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    const char* server_ip = "18.209.46.251"; // Replace with your server IP

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "[CLIENT] Socket creation failed: " << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "[CLIENT] Successfully created socket (fd: " << sockfd << ")" << std::endl;

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        std::cerr << "[CLIENT] Invalid address/Address not supported: " << server_ip << std::endl;
        close(sockfd);
        return EXIT_FAILURE;
    }

    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "[CLIENT] Connection failed: " << strerror(errno) << std::endl;
        close(sockfd);
        return EXIT_FAILURE;
    }

    std::cout << "[CLIENT] Successfully connected to server at " << server_ip << ":" << PORT << std::endl;

    // Example communication
    const char* message = "Hello from client";
    if (send(sockfd, message, strlen(message), 0) < 0) {
        std::cerr << "[CLIENT] Send failed: " << strerror(errno) << std::endl;
    }

    char buffer[1024] = {0};
    ssize_t bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
    if (bytes_read > 0) {
        std::cout << "[CLIENT] Received: " << buffer << std::endl;
    }

    close(sockfd);
    return EXIT_SUCCESS;
}
