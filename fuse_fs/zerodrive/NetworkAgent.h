#pragma once

#include <functional>
#include <set>
#include "Protocol.h"
#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <netinet/in.h>

class DriveAgent;

class NetworkAgent {
public:
    enum Role {
        UNKNOWN,
        SERVER,
        CLIENT
    };

    Role getRole() const;

    void setRole(Role role);

private:
    DriveAgent *hostDriveAgent = nullptr;
    Role role = UNKNOWN;
    std::set<int> connections;
    std::thread *listeningThread = nullptr;
    std::thread *connectingThread = nullptr;
    bool shutDown = false;
    int listenPort = 0;
    std::vector<std::thread*> messageLoopThreadPool;
public:
    NetworkAgent(DriveAgent *);

    ~NetworkAgent();

    bool isConnected();

    int listenAsync(const char *address, int port, std::function<void()> callback);

    int connectAsync(const char *address, int port, std::function<void()> callback);

    int connectSync(struct sockaddr_in, std::function<void()> callback);

    int listenSync(struct sockaddr_in, std::function<void()> callback);

    int sendRaw(int socket_fd, const char *data, int length);

    int sendMessage(int socket_fd, enum Message msg, const std::vector<std::string> &detail);

    int sendMessageToAll(enum Message msg, const std::vector<std::string> &detail);

    int disconnect();

    int requestFile(std::string path);

    void sendRequestedFile(int fd, std::string basicString);

    void freeSocket();

private:
    void error(const std::string &msg) {
        std::cerr << "[Error] " << msg << std::endl;
    }

    void MessageLoop(int sockfd);

    int readBytes(int fd, char *buffer, int length);

    int32_t readInt32(int fd);

    std::string readString(int fd);

    sockaddr_in _server_addr{}; // a record of server address
};

