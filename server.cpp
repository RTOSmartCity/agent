#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

class MessageHandlerServer {
private:
    int serverSocket;
    std::map<std::string, std::string> credentials;  // username -> password
    std::map<int, std::string> authenticatedClients; // socket -> username
    std::map<int, std::unique_ptr<std::mutex>> socketMutexes; // socket -> mutex for writing
    std::mutex clientsMutex;

    void handleClient(int clientSocket) {
        // Buffer for reading data
        char buffer[1024] = {0};
        
        // Handle authentication
        int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) {
            close(clientSocket);
            return;
        }
        
        std::string authMessage(buffer, bytesRead);
        size_t delimiterPos = authMessage.find('|');
        if (delimiterPos == std::string::npos) {
            write(clientSocket, "AUTH_FAILURE", 12);
            close(clientSocket);
            return;
        }
        
        std::string username = authMessage.substr(0, delimiterPos);
        std::string password = authMessage.substr(delimiterPos + 1);
        if (credentials.count(username) && credentials[username] == password) {
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                authenticatedClients[clientSocket] = username;
                socketMutexes[clientSocket] = std::make_unique<std::mutex>();
            }
            write(clientSocket, "AUTH_SUCCESS", 12);
            std::cout << "Client " << username << " authenticated\n";
        } else {
            write(clientSocket, "AUTH_FAILURE", 12);
            close(clientSocket);
            return;
        }

        // Main loop to handle messages
        while (true) {
            char msgBuffer[1024] = {0};
            bytesRead = read(clientSocket, msgBuffer, sizeof(msgBuffer) - 1);
            if (bytesRead > 0) {
                std::string message(msgBuffer, bytesRead);
                std::string relayedMessage = username + ":" + message;
                
                // Relay message to all authenticated clients
                std::vector<int> targets;
                {
                    std::lock_guard<std::mutex> lock(clientsMutex);
                    for (const auto& pair : authenticatedClients) {
                        targets.push_back(pair.first);
                    }
                }
                for (int target : targets) {
                    auto it = socketMutexes.find(target);
                    if (it != socketMutexes.end()) {
                        std::lock_guard<std::mutex> lock(*it->second);
                        if (write(target, relayedMessage.c_str(), relayedMessage.size()) < 0) {
                            std::cerr << "Error writing to socket " << target << "\n";
                        }
                    }
                }
            } else if (bytesRead == 0) {
                // Client disconnected
                std::cout << "Client " << username << " disconnected\n";
                {
                    std::lock_guard<std::mutex> lock(clientsMutex);
                    authenticatedClients.erase(clientSocket);
                    socketMutexes.erase(clientSocket);
                }
                close(clientSocket);
                return;
            }
        }
    }

public:
    MessageHandlerServer(int port) {
        // Hardcoded credentials for simplicity
        credentials["vehicle1"] = "pass123";
        credentials["pedestrian1"] = "pass456";
        credentials["traffic1"] = "pass789";

        // Create server socket
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            std::cerr << "Error creating socket\n";
            exit(1);
        }

        // Bind to port
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Error binding to port\n";
            exit(1);
        }

        // Listen for connections
        if (listen(serverSocket, 5) < 0) {
            std::cerr << "Error listening\n";
            exit(1);
        }
        std::cout << "Server listening on port " << port << "\n";
    }

    void run() {
        while (true) {
            sockaddr_in clientAddr{};
            socklen_t clientLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
            if (clientSocket < 0) {
                std::cerr << "Error accepting connection\n";
                continue;
            }
            std::thread clientThread(&MessageHandlerServer::handleClient, this, clientSocket);
            clientThread.detach();
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