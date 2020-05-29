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

bool NetworkAgent::isConnected() {
    return !connections.empty();
}

int NetworkAgent::sendMessage(int socket_fd, enum Message msg, const std::vector<std::string> &detail) {
    int32_t msg_i = msg;
    int32_t string_cnt = detail.size();
    sendRaw(socket_fd, (const char *) &msg_i, sizeof(msg_i));
    sendRaw(socket_fd, (const char *) &string_cnt, sizeof(string_cnt));
    for (int i = 0; i < detail.size(); ++i) {
        int32_t length = detail[i].size();
        sendRaw(socket_fd, (const char *) &length, sizeof(length));
        sendRaw(socket_fd, detail[i].c_str(), sizeof(length));
    }
    return 0;
}

int NetworkAgent::sendRaw(int socket_fd, const char *data, int length) {
    int n = write(socket_fd, data, length);
    if (n != length) {
        std::cout << "Error sending to " << socket_fd << std::endl;
    }
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
}

int NetworkAgent::listenSync(struct sockaddr_in server_addr, std::function<void()> callback) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        error("Cannot open socket");
        return 1;
    }
    if (bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error("Cannot bind socket");
        return 1;
    }
    listen(socket_fd,8);
    struct sockaddr_in client_addr{};
    socklen_t client_len =sizeof(client_addr);
    int newsockfd = accept(socket_fd, (struct sockaddr*)& client_addr, &client_len);
    if(newsockfd<0)
        error("Error on accept");
    connections.insert(newsockfd);
    callback();
    return 0;
}

