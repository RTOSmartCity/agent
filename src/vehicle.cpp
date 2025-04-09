#include "messagehandler.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./vehicle <config_file>\n";
        return 1;
    }
    std::string configFile = argv[1];

    // Read config file
    std::ifstream config(configFile);
    if (!config.is_open()) {
        std::cerr << "Error opening config file: " << configFile << "\n";
        return 1;
    }
    std::map<std::string, std::string> configMap;
    std::string line;
    while (std::getline(config, line)) {
        size_t eqPos = line.find('=');
        if (eqPos != std::string::npos) {
            std::string key = line.substr(0, eqPos);
            std::string value = line.substr(eqPos + 1);
            configMap[key] = value;
        }
    }
    config.close();

    // Extract required config values
    if (configMap.find("username") == configMap.end() || 
        configMap.find("password") == configMap.end() || 
        configMap.find("x") == configMap.end() || 
        configMap.find("y") == configMap.end()) {
        std::cerr << "Config file must contain username, password, x, and y\n";
        return 1;
    }
    std::string username = configMap["username"];
    std::string password = configMap["password"];
    double x = std::stod(configMap["x"]);
    double y = std::stod(configMap["y"]);

    // Connect to server
    const char* envIp = std::getenv("SERVER_IP");
    std::string serverIp = envIp ? envIp : "127.0.0.1";
    const int serverPort = 8080;
    std::cout << "Connecting to server at " << serverIp << ":" << serverPort << "\n";
    MessageHandler handler(serverIp, serverPort);

    // Authenticate
    std::cout << "Authenticating " << username << "...\n";
    if (!handler.authenticate(username, password)) {
        std::cerr << "Authentication failed for " << username << "\n";
        return 1;
    }

    // Main loop
    while (true) {
        // Simulate movement by updating position
        x += 1.0;
        y += 1.0;
        std::string message = "POSITION|" + std::to_string(x) + "|" + std::to_string(y);
        handler.sendMessage(message);

        // Process received messages
        while (true) {
            std::string msg = handler.getMessage();
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

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}