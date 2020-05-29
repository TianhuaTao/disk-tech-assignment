#pragma once
#include <functional>
#include <set>
#include "Protocol.h"
#include <vector>
#include <string>
#include <iostream>
class NetworkAgent {
public:
    enum Role { UNKNOWN,
                SERVER,
                CLIENT };

private:
    Role role = UNKNOWN;
    std::set<int> connections;
public:
    NetworkAgent();
    ~NetworkAgent();

    bool isConnected();
    int listenAsync(const char *address, int port, std::function<void()> callback);
    int connectAsync(const char *address, int port, std::function<void()> callback);
    int connectSync();
    int listenSync(struct sockaddr_in, std::function<void()> callback);
    int sendRaw(int socket_fd, const char*data, int length);
    int sendMessage(int socket_fd, enum Message msg, const std::vector<std::string> &detail);
    int sendMessageToAll( enum Message msg, const std::vector<std::string>& detail);


private:
    void error(const std::string& msg){
        std::cout<<msg<<std::endl;
    }
};

