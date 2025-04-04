#define _QNX_SOURCE 1  // Must be first
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdlib>      // For EXIT_FAILURE/SUCCESS

#define PORT 8080

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    const char* server_ip = "18.209.46.251"; // Use actual server IP

    // Create socket with QNX-specific protocol
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        std::cerr << "[CLIENT] Socket creation failed: " << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "[CLIENT] Created socket (fd: " << sockfd << ")" << std::endl;

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // QNX-compatible IP conversion
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        std::cerr << "[CLIENT] Invalid address: " << server_ip << " (" << strerror(errno) << ")" << std::endl;
        close(sockfd);
        return EXIT_FAILURE;
    }

    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "[CLIENT] Connection failed: " << strerror(errno) << std::endl;
        close(sockfd);
        return EXIT_FAILURE;
    }

    std::cout << "[CLIENT] Connected to " << server_ip << ":" << PORT << std::endl;

    // QNX-compatible communication
    const char* message = "Hello from QNX client";
    ssize_t bytes_sent = write(sockfd, message, strlen(message));
    if (bytes_sent < 0) {
        std::cerr << "[CLIENT] Write failed: " << strerror(errno) << std::endl;
    } else {
        std::cout << "[CLIENT] Sent " << bytes_sent << " bytes" << std::endl;
    }

    char buffer[1024] = {0};
    ssize_t bytes_read = read(sockfd, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        std::cout << "[CLIENT] Received: " << buffer << std::endl;
    } else if (bytes_read == 0) {
        std::cout << "[CLIENT] Server closed connection" << std::endl;
    } else {
        std::cerr << "[CLIENT] Read error: " << strerror(errno) << std::endl;
    }

    close(sockfd);
    return EXIT_SUCCESS;
}
