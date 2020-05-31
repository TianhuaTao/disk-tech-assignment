#pragma once
#include <functional>
#include <set>
#include "Protocol.h"
#include <vector>
#include <string>
#include <iostream>
#include <thread>
class NetworkAgent {
public:
    enum Role { UNKNOWN,
                SERVER,
                CLIENT };

private:
    Role role = UNKNOWN;
public:
    Role getRole() const;

    void setRole(Role role);

private:
    std::set<int> connections;
    std::thread* listeningThread = nullptr;
    std::thread* connectingThread = nullptr;
    bool shutDown = false;
public:
    NetworkAgent();
    ~NetworkAgent();

    bool isConnected();
    int listenAsync(const char *address, int port, std::function<void()> callback);
    int connectAsync(const char *address, int port, std::function<void()> callback);
    int connectSync(struct sockaddr_in, std::function<void()> callback);
    int listenSync(struct sockaddr_in, std::function<void()> callback);
    int sendRaw(int socket_fd, const char*data, int length);
    int sendMessage(int socket_fd, enum Message msg, const std::vector<std::string> &detail);
    int sendMessages(std::set<int>& connections, enum Message msg, const std::vector<std::string> &detail);
    int sendMessageToAll( enum Message msg, const std::vector<std::string>& detail);
    int disconnect();

private:
    void error(const std::string& msg){
        std::cout<<msg<<std::endl;
    }
    void MessageLoop(int sockfd);
    int readBytes(int fd, char* buffer, int length);
    int32_t readInt32(int fd);
    std::string readString(int fd );
};

