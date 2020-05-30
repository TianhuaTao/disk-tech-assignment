//
// Created by sam on 2020/5/30.
//
#include "NetworkAgent.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <cstdint>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>

bool NetworkAgent::isConnected() {
    return !connections.empty();
}

int NetworkAgent::sendMessage(int socket_fd, enum Message msg, const std::vector<std::string> &detail) {
    printf("Send message to %d\n", socket_fd);
    int32_t msg_i = msg;
    int32_t string_cnt = detail.size();
    sendRaw(socket_fd, (const char *) &msg_i, sizeof(msg_i));
    sendRaw(socket_fd, (const char *) &string_cnt, sizeof(string_cnt));
    for (const auto &i : detail) {
        int32_t length = i.size();
        sendRaw(socket_fd, (const char *) &length, sizeof(length));
        std::cout<< i.c_str()<<std::endl;   // debug
        sendRaw(socket_fd, i.c_str(), length);
    }
    return 0;
}

int NetworkAgent::sendRaw(int socket_fd, const char *data, int length) {
    int n = write(socket_fd, data, length);
    if (n != length) {
        std::cout << "Error sending to " << socket_fd << std::endl;
    }
//    printf("[debug] send %d bytes\n", n);
    return 0;
}

int NetworkAgent::sendMessageToAll(enum Message msg, const std::vector<std::string> &detail) {
    for (int fd:connections) {
        sendMessage(fd, msg, detail);
    }
    return 0;
}

int NetworkAgent::listenAsync(const char *address, int port, std::function<void()> callback) {
    struct sockaddr_in sa{};
    sa.sin_family = AF_INET;
//    inet_pton(AF_INET,address,&(sa.sin_addr));
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(port);

    // TODO: use thread pool to call listenSync
    listeningThread = new std::thread(&NetworkAgent::listenSync, this, sa, callback);
    return 0;
}

int NetworkAgent::listenSync(struct sockaddr_in server_addr, std::function<void()> callback) {
    std::cout << "Start listenSync\n";

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        error("Cannot open socket");
        return 1;
    }
    if (bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error("Cannot bind socket");
        return 1;
    }

    listen(socket_fd, 8);
    while (connections.size() <= 8) {
        struct sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int newsockfd = accept(socket_fd, (struct sockaddr *) &client_addr, &client_len);
        printf("New connection\n");
        if (newsockfd < 0)
            error("Error on accept");
        connections.insert(newsockfd);
        callback();
        MessageLoop(newsockfd); // TODO: BUG -- use another thread
    }
    return 0;
}

NetworkAgent::NetworkAgent() {
    printf("New NetworkAgent...\n");
}

NetworkAgent::~NetworkAgent() {
    delete listeningThread;
}

int NetworkAgent::connectAsync(const char *address, int port, std::function<void()> callback) {
    struct sockaddr_in sa{};
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, address, &(sa.sin_addr));
//    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(port);

    connectingThread = new std::thread(&NetworkAgent::connectSync, this, sa, callback);

    return 0;
}

int NetworkAgent::connectSync(struct sockaddr_in server_addr, std::function<void()> callback) {
    std::cout << "Start connectSync\n";
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        error("Cannot open socket");
        return 1;
    }
    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error("Cannot connect");
        // TODO: try later
        return 1;
    }
    printf("Made connection\n");

    connections.insert(socket_fd);
    callback();
    MessageLoop(socket_fd);
    return 0;
}

NetworkAgent::Role NetworkAgent::getRole() const {
    return role;
}

void NetworkAgent::setRole(NetworkAgent::Role role) {
    NetworkAgent::role = role;
}

// TODO: complete the function
void NetworkAgent::MessageLoop(int sockfd) {
    printf("Start message loop\n");
    while (!shutDown) {
        // read message token
        enum Message token;
        int32_t token_i;
        int ret = readBytes(sockfd, (char *) &token_i, sizeof(int32_t));
        if (ret < 0) break;
        token = (Message) token_i;


        if (token==WRITE_DONE) {
            std::cout << "Receive: WRITE_DONE, ";
            int32_t string_cnt = readInt32(sockfd);
//            std::cout << "argc="<<string_cnt<<std::endl;
            if (string_cnt != 1)error("Wrong value");
            auto path = readString(sockfd);
            std::cout << "path="<<path<<std::endl;
            // TODO: more operation
        }
        else if(token==RENAME){

        }

        else{

        }
    }

    printf("Stop message loop\n");

}

int NetworkAgent::readBytes(int fd, char *buffer, int length) {
    int bytesRead = 0;
    int result;
    while (bytesRead < length) {
        result = read(fd, buffer + bytesRead, length - bytesRead);
        if (result < 1) {
            error("Read bytes error");
            return -1;
        }
        bytesRead += result;
    }
//    printf("[debug] read %d bytes\n", result);
    return result;
}

int32_t NetworkAgent::readInt32(int fd) {
    int32_t ret;
    readBytes(fd, (char *) &ret, sizeof(int32_t));
    return ret;
}

std::string NetworkAgent::readString(int fd ) {
    int32_t length = readInt32(fd);
//    printf("[debug] string length: %d\n", length);

    char *buf = new char[length + 1]();
    if (length > 0)
        readBytes(fd, buf, length);
    std::string s(buf);
    delete[]buf;
    return s;
}

