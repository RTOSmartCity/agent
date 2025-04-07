#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class MessageHandler {
private:
    int clientSocket;
    bool authenticated;

public:
    MessageHandler(const std::string& serverIp, int serverPort) : authenticated(false) {
        // Create client socket
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket < 0) {
            std::cerr << "Error creating socket" << std::endl;
            exit(1);
        }

        // Connect to server
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(serverPort);
        serverAddr.sin_addr.s_addr = inet_addr(serverIp.c_str());
        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Error connecting to server" << std::endl;
            exit(1);
        }
        std::cout << "Connected to server at " << serverIp << ":" << serverPort << std::endl;
    }

    bool authenticate(const std::string& username, const std::string& password) {
        std::string authMessage = username + "|" + password;
        write(clientSocket, authMessage.c_str(), authMessage.size());
        char buffer[1024] = {0};
        int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            std::string response(buffer, bytesRead);
            if (response == "AUTH_SUCCESS") {
                authenticated = true;
                return true;
            }
        }
        return false;
    }

    bool sendMessage(const std::string& message) {
        if (!authenticated) {
            std::cerr << "Not authenticated" << std::endl;
            return false;
        }
        write(clientSocket, message.c_str(), message.size());
        return true;
    }

    std::string receiveMessage() {
        if (!authenticated) {
            return "";
        }
        char buffer[1024] = {0};
        int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            return std::string(buffer, bytesRead);
        }
        return "";
    }

    ~MessageHandler() {
        close(clientSocket);
    }
};

int main() {
    MessageHandler client("127.0.0.1", 8080);
    if (client.authenticate("vehicle1", "pass123")) {
        std::cout << "Authentication successful" << std::endl;
        client.sendMessage("Hello from vehicle1");
        std::string msg = client.receiveMessage();
        if (!msg.empty()) {
            std::cout << "Received: " << msg << std::endl;
        }
    } else {
        std::cout << "Authentication failed" << std::endl;
    }
    return 0;
}