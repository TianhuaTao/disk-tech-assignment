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

    uint64_t pullfromServer(uint64_t last_sync);

    void sendAllData(int fd);

    void pushToClient(int fd, const std::set<std::string>& set, const std::set<std::string>& set1, const std::set<std::string>& set2,
                      const std::set<std::string>& set3, const std::set<std::pair<std::string, std::string>>& set4);

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
    int sendToken(int socket_fd, Operation_t token);

    int sendMessage(int socket_fd, enum Operation_t msg, const std::vector<std::string> &detail);

//    int sendMessageToAll(enum Operation_t msg, const std::vector<std::string> &detail);

    int disconnect();

//    int requestFile(std::string path);

//    void sendRequestedFile(int fd, std::string path);
    void uploadFile(int remote_fd, const std::string& path);
    void downloadFile(int remote_fd, const std::string& path);
    void freeSocket();
    uint64_t pushToServer(std::set<std::string > &newFiles , std::set<std::string > &deleteFiles,
                          std::set<std::string > &newDirs, std::set<std::string > &deleteDirs,
                          std::set<std::pair<std::string,std::string>> &renameDirs);
    int readBytes(int fd, char *buffer, int length);

    int32_t readInt32(int fd);
    uint64_t readUint64(int fd);
    std::string readString(int fd);

private:
    void error(const std::string &msg) {
        std::cout << "[Error] " << msg << std::endl;
    }

    void MessageLoop(int sockfd);


    sockaddr_in _server_addr{}; // a record of server address
    enum Operation_t readToken(int sockfd);
};

