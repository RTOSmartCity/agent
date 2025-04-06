#include <iostream>
#include <string>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

class MessageHandlerServer {
private:
    int serverSocket;
    std::map<std::string, std::string> credentials;  // username -> password
    std::map<int, std::string> authenticatedClients; // socket -> username

public:
    MessageHandlerServer(int port) {
        // Hardcoded credentials for simplicity
        credentials["vehicle1"] = "pass123";
        credentials["pedestrian1"] = "pass456";
        credentials["traffic1"] = "pass789";

        // Create server socket
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            std::cerr << "Error creating socket" << std::endl;
            exit(1);
        }

        // Bind to port
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Error binding to port" << std::endl;
            exit(1);
        }

        // Listen for connections
        if (listen(serverSocket, 5) < 0) {
            std::cerr << "Error listening" << std::endl;
            exit(1);
        }
        std::cout << "Server listening on port " << port << std::endl;
    }

    void run() {
        while (true) {
            // Accept new connections
            sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
            if (clientSocket < 0) {
                std::cerr << "Error accepting connection" << std::endl;
                continue;
            }

            // Handle authentication
            char buffer[1024] = {0};
            int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
            if (bytesRead > 0) {
                std::string authMessage(buffer, bytesRead);
                size_t delimiterPos = authMessage.find('|');
                if (delimiterPos != std::string::npos) {
                    std::string username = authMessage.substr(0, delimiterPos);
                    std::string password = authMessage.substr(delimiterPos + 1);
                    if (credentials.count(username) && credentials[username] == password) {
                        authenticatedClients[clientSocket] = username;
                        write(clientSocket, "AUTH_SUCCESS", 12);
                        std::cout << "Client " << username << " authenticated" << std::endl;
                    } else {
                        write(clientSocket, "AUTH_FAILURE", 12);
                        close(clientSocket);
                    }
                }
            }

            // Handle messages from authenticated clients
            for (auto it = authenticatedClients.begin(); it != authenticatedClients.end();) {
                int clientSocket = it->first;
                char msgBuffer[1024] = {0};
                int bytesRead = read(clientSocket, msgBuffer, sizeof(msgBuffer) - 1);
                if (bytesRead > 0) {
                    std::string message(msgBuffer, bytesRead);
                    std::cout << "Received from " << it->second << ": " << message << std::endl;
                    // Relay message to all other clients
                    for (const auto& otherClient : authenticatedClients) {
                        if (otherClient.first != clientSocket) {
                            write(otherClient.first, message.c_str(), message.size());
                        }
                    }
                    ++it;
                } else if (bytesRead == 0) {
                    // Client disconnected
                    std::cout << "Client " << it->second << " disconnected" << std::endl;
                    close(clientSocket);
                    it = authenticatedClients.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    ~MessageHandlerServer() {
        close(serverSocket);
    }
};

int main() {
    MessageHandlerServer server(8080);
    server.run();
    return 0;
}