#define _QNX_SOURCE
#define _POSIX_C_SOURCE 200112L
#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class MessageHandler {
private:
    int clientSocket;
    bool authenticated;
    std::string username;
    std::thread receiverThread;
    std::queue<std::string> messageQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondVar;
    bool running;

    void receiverFunction() {
        while (running) {
            char buffer[1024] = {0};
            int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
            if (bytesRead > 0) {
                std::string message(buffer, bytesRead);
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    messageQueue.push(message);
                }
                queueCondVar.notify_one();
            } else if (bytesRead == 0) {
                std::cout << "Server disconnected\n";
                running = false;
            }
        }
    }

public:
    MessageHandler(const std::string& serverIp, int serverPort) 
        : authenticated(false), running(true) {
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket < 0) {
            throw std::runtime_error("Error creating socket");
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(serverPort);
        serverAddr.sin_addr.s_addr = inet_addr(serverIp.c_str());
        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            close(clientSocket);
            throw std::runtime_error("Error connecting to server");
        }
        std::cout << "Connected to server at " << serverIp << ":" << serverPort << "\n";
        
        receiverThread = std::thread(&MessageHandler::receiverFunction, this);
    }

    ~MessageHandler() {
        running = false;
        close(clientSocket);
        if (receiverThread.joinable()) {
            receiverThread.join();
        }
    }

    bool authenticate(const std::string& username, const std::string& password) {
        std::string authMessage = username + "|" + password;
        if (write(clientSocket, authMessage.c_str(), authMessage.size()) < 0) {
            return false;
        }
        
        std::string response = getMessage();
        if (response == "AUTH_SUCCESS") {
            authenticated = true;
            this->username = username;
            return true;
        }
        return false;
    }

    void sendMessage(const std::string& message) {
        if (!authenticated) {
            throw std::runtime_error("Not authenticated");
        }
        if (write(clientSocket, message.c_str(), message.size()) < 0) {
            std::cerr << "Error sending message\n";
        }
    }

    std::string getMessage() {
        std::unique_lock<std::mutex> lock(queueMutex);
        queueCondVar.wait(lock, [this]{ return !messageQueue.empty() || !running; });
        if (!messageQueue.empty()) {
            std::string msg = messageQueue.front();
            messageQueue.pop();
            return msg;
        }
        return "";
    }

    std::string getUsername() const { return username; }
};

class Agent {
protected:
    MessageHandler handler;
    std::string username;

public:
    Agent(const std::string& serverIp, int serverPort, 
          const std::string& username, const std::string& password)
        : handler(serverIp, serverPort), username(username) {
        if (!handler.authenticate(username, password)) {
            throw std::runtime_error("Authentication failed for " + username);
        }
    }

    void sendMessage(const std::string& message) {
        handler.sendMessage(message);
    }

    std::string getMessage() {
        return handler.getMessage();
    }
};

class Vehicle : public Agent {
private:
    double x, y;  // Position

public:
    Vehicle(const std::string& serverIp, int serverPort, 
            const std::string& username, const std::string& password)
        : Agent(serverIp, serverPort, username, password), x(0.0), y(0.0) {}

    void updatePosition(double newX, double newY) {
        x = newX;
        y = newY;
        std::string message = "POSITION|" + std::to_string(x) + "|" + std::to_string(y);
        sendMessage(message);
    }

    void update() {
        // Simulate movement (e.g., increment position)
        updatePosition(x + 1.0, y + 1.0);

        // Process received messages
        while (true) {
            std::string msg = getMessage();
            if (msg.empty()) break;
            size_t colonPos = msg.find(':');
            if (colonPos != std::string::npos) {
                std::string sender = msg.substr(0, colonPos);
                std::string content = msg.substr(colonPos + 1);
                if (sender != username) {
                    std::cout << username << " received from " << sender << ": " << content << "\n";
                }
            }
        }
    }
};

int main() {
    try {
        const char* envIp = std::getenv("SERVER_IP");
        std::string serverIp = envIp ? envIp : "127.0.0.1";
        const int serverPort = 8080;

        Vehicle vehicle1(serverIp, serverPort, "vehicle1",   "pass123");
        Vehicle vehicle2(serverIp, serverPort, "pedestrian1","pass456");

        // Simulate CV2X communication
        for (int i = 0; i < 5; ++i) {
            vehicle1.update();
            vehicle2.update();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}
