#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#ifdef __QNX__
#define _QNX_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif // __QNX__

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

    void receiverFunction();

public:
    MessageHandler(const std::string& serverIp, int serverPort);
    ~MessageHandler();
    bool authenticate(const std::string& username, const std::string& password);
    void sendMessage(const std::string& message);
    std::string getMessage();
    std::string waitForMessage();
    std::string getUsername() const;
};

#endif // MESSAGEHANDLER_H