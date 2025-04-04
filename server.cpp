#define _QNX_SOURCE 1
#include <sys/neutrino.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX         80
#define PORT        8080

std::string bigEndianToIPAndPort(uint32_t big_endian_ip, uint16_t port) {
    uint32_t host_ip = ntohl(big_endian_ip);
    unsigned char bytes[4];
    bytes[0] = (host_ip >> 24) & 0xFF;
    bytes[1] = (host_ip >> 16) & 0xFF;
    bytes[2] = (host_ip >> 8) & 0xFF;
    bytes[3] = host_ip & 0xFF;
    return std::to_string(bytes[0]) + "." + std::to_string(bytes[1]) + "." +
           std::to_string(bytes[2]) + "." + std::to_string(bytes[3]) + ":" + std::to_string(port);
}

int main() {
    char buff[MAX];
    int sockfd, connfd;
    socklen_t len;
    struct sockaddr_in address, cli;

    // Create socket without any special options
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "[SERVER] socket failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    // Bind configuration
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "[SERVER] bind failed: " << strerror(errno) << std::endl;
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 5) < 0) {
        std::cerr << "[SERVER] listen failed: " << strerror(errno) << std::endl;
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    std::cout << "[SERVER] Listening on port " << PORT << std::endl;

    while (true) {
        len = sizeof(cli);
        if ((connfd = accept(sockfd, (struct sockaddr*)&cli, &len)) < 0) {
//            std::cerr << "[SERVER] accept failed: " << strerror(errno) << std::endl;
            continue;
        }

        std::cout << "[SERVER] Accepted connection from: "
                  << bigEndianToIPAndPort(cli.sin_addr.s_addr, cli.sin_port) << std::endl;

        ssize_t bytes_read;
        while ((bytes_read = read(connfd, buff, sizeof(buff)-1)) > 0) {
            buff[bytes_read] = '\0';
            std::cout << "[SERVER] Received: " << buff << std::endl;

            const char* response = "ACK from QNX";
            if (write(connfd, response, strlen(response)) < 0) {
                std::cerr << "[SERVER] write error: " << strerror(errno) << std::endl;
            }
        }

        close(connfd);
    }

    close(sockfd);
    return 0;
}
