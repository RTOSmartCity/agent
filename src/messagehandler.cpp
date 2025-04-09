#include "messagehandler.h"
#include <iostream>
#include <stdexcept>

MessageHandler::MessageHandler(const std::string& serverIp, int serverPort) 
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

MessageHandler::~MessageHandler() {
    running = false;
    close(clientSocket);
    if (receiverThread.joinable()) {
        receiverThread.join();
    }
}

void MessageHandler::receiverFunction() {
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

bool MessageHandler::authenticate(const std::string& username, const std::string& password) {
    std::string authMessage = username + "|" + password;
    if (write(clientSocket, authMessage.c_str(), authMessage.size()) < 0) {
        std::cerr << "Failed to send authentication message\n";
        return false;
    }
    
    std::string response = waitForMessage();
    if (response == "AUTH_SUCCESS") {
        authenticated = true;
        this->username = username;
        std::cout << "Authentication successful for " << username << "\n";
        return true;
    } else {
        std::cerr << "Error: Authentication failed for " << username << "\n";
        return false;
    }
}

void MessageHandler::sendMessage(const std::string& message) {
    if (!authenticated) {
        throw std::runtime_error("Not authenticated");
    }
    if (write(clientSocket, message.c_str(), message.size()) < 0) {
        std::cerr << "Error sending message\n";
    }
}

std::string MessageHandler::getMessage() {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (!messageQueue.empty()) {
        std::string msg = messageQueue.front();
        messageQueue.pop();
        return msg;
    }
    return "";
}

std::string MessageHandler::waitForMessage() {
    std::unique_lock<std::mutex> lock(queueMutex);
    queueCondVar.wait(lock, [this]{ return !messageQueue.empty() || !running; });
    if (!messageQueue.empty()) {
        std::string msg = messageQueue.front();
        messageQueue.pop();
        return msg;
    }
    return "";
}

std::string MessageHandler::getUsername() const {
    return username;
}