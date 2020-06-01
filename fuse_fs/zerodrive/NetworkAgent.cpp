//
// Created by sam on 2020/5/30.
//
#define FUSE_USE_VERSION 31

#include "Protocol.h"
#include "NetworkAgent.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <cstdint>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <cstdio>
#include "DriveAgent.h"
#include "op.h"

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
        std::cout << i.c_str() << std::endl;   // debug
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
    if (listeningThread) {
        error("NetworkAgent is already listening");
    }

    this->listenPort = port;
    struct sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(port);

    std::cerr << "[NetworkAgent::listenAsync] port = " << port << " " << sa.sin_port << std::endl;

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
        std::cerr << socket_fd << " "
                  //<<server_addr.sin_addr<<" "
                  << server_addr.sin_family << " "
                  << server_addr.sin_port << " "
                  << server_addr.sin_zero
                  << std::endl;
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

        messageLoopThreadPool.push_back(new std::thread(&NetworkAgent::MessageLoop, this, newsockfd)) ;
    }
    return 0;
}

NetworkAgent::NetworkAgent(DriveAgent *host) : hostDriveAgent(host) {
    printf("New NetworkAgent...\n");
}

NetworkAgent::~NetworkAgent() {
    for (int c: connections) {
        close(c);
    }
    delete listeningThread;
}

int NetworkAgent::connectAsync(const char *address, int port, std::function<void()> callback) {
    struct sockaddr_in sa{};
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, address, &(sa.sin_addr));
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
    this->_server_addr = server_addr;
    connections.insert(socket_fd);
    callback();
    MessageLoop(socket_fd);
    return 0;
}

NetworkAgent::Role NetworkAgent::getRole() const {
    return role;
}

void NetworkAgent::setRole(NetworkAgent::Role newRole) {
    NetworkAgent::role = newRole;
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

        if (token == WRITE_DONE) {
            std::cout << "Receive: WRITE_DONE, ";
            int32_t string_cnt = readInt32(sockfd);
//            std::cout << "argc="<<string_cnt<<std::endl;
            if (string_cnt != Message_Number[token])error("Wrong value");
            auto path = readString(sockfd);
            std::cout << "path=" << path << std::endl;

            hostDriveAgent->onMsgWriteDone(path);
        } else if (token == RENAME) {
            std::cout << "Receive: RENAME, ";
            int32_t string_cnt = readInt32(sockfd);
//            std::cout << "argc="<<string_cnt<<std::endl;
            if (string_cnt != Message_Number[token])error("Wrong value");
            auto from = readString(sockfd);
            std::cout << "from=" << from << std::endl;
            auto to = readString(sockfd);
            std::cerr << "to=" << to << std::endl;

            hostDriveAgent->onMsgRename(from, to);

        } else if (token == CREATE) {
            std::cout << "Receive: CREATE, ";
            int32_t string_cnt = readInt32(sockfd);
//            std::cout << "argc="<<string_cnt<<std::endl;
            if (string_cnt != Message_Number[token])error("Wrong value");
            auto path = readString(sockfd);
            std::cout << "path=" << path << std::endl;
            auto mode = (mode_t) std::stoi(readString(sockfd));
            std::cout << "mode=" << mode << std::endl;

            hostDriveAgent->onMsgCreate(path, mode);

        } else if (token == MKDIR) {
            std::cout << "Receive: MKDIR, ";
            int32_t string_cnt = readInt32(sockfd);
//            std::cout << "argc="<<string_cnt<<std::endl;
            if (string_cnt != Message_Number[token])error("Wrong value");//mkdir has 2 messages
            auto path = readString(sockfd);
            std::cout << "path=" << path << std::endl;
            auto mode = (mode_t) std::stoi(readString(sockfd));
            std::cout << "mode=" << mode << std::endl;

            hostDriveAgent->onMsgMkdir(path, mode);
        } else if (token == RMDIR) {
            std::cout << "Receive: RMDIR, ";
            int32_t string_cnt = readInt32(sockfd);
//            std::cout << "argc="<<string_cnt<<std::endl;
            if (string_cnt != Message_Number[token])error("Wrong value");//rmdir has 1 messages
            auto path = readString(sockfd);
            std::cout << "path=" << path << std::endl;

            hostDriveAgent->onMsgRmdir(path);

        } else if (token == CHMOD) {
            std::cout << "Receive: CHMOD, ";
            int32_t string_cnt = readInt32(sockfd);
//            std::cout << "argc="<<string_cnt<<std::endl;
            if (string_cnt != Message_Number[token])error("Wrong value");
            auto path = readString(sockfd);
            std::cout << "path=" << path << std::endl;
            auto mode = (mode_t) std::stoi(readString(sockfd));
            std::cout << "mode=" << mode << std::endl;

            hostDriveAgent->onMsgChmod(path, mode);

        } else if (token == CHOWN) {
            std::cout << "Receive: CHOWN, ";
            int32_t string_cnt = readInt32(sockfd);
//            std::cout << "argc="<<string_cnt<<std::endl;
            if (string_cnt != Message_Number[token])error("Wrong value");
            auto path = readString(sockfd);
            std::cout << "path=" << path << std::endl;
            auto from = (mode_t) std::stoi(readString(sockfd));
            std::cout << "mode=" << from << std::endl;
            auto to = (mode_t) std::stoi(readString(sockfd));
            std::cout << "mode=" << to << std::endl;

            hostDriveAgent->onMsgChown();
        } else if (token == REQUEST_FILE) {
            std::cout << "Receive: REQUEST_FILE, ";
            int32_t string_cnt = readInt32(sockfd);
            if (string_cnt != Message_Number[token])error("Wrong value");
            auto path = readString(sockfd);
            std::cout << "path=" << path << std::endl;
            sendRequestedFile(sockfd, path);
        } else {
            std::cout << "Can't recognize the token..." << token << std::endl;
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

std::string NetworkAgent::readString(int fd) {
    int32_t length = readInt32(fd);
//    printf("[debug] string length: %d\n", length);

    char *buf = new char[length + 1]();
    if (length > 0)
        readBytes(fd, buf, length);
    std::string s(buf);
    delete[]buf;
    return s;
}

int NetworkAgent::disconnect() {
    printf("NetworkAgent::disconnect() is not implemented\n");
    return 0;
}

// request a file from server
// first store the file in tmp dir
// then move it the target location
int NetworkAgent::requestFile(std::string path) {
    if (role == SERVER)error("server should not request file");
    if (connections.empty()) { return -1; }
    int server_fd = *connections.begin();
    std::vector<std::string> detail;
    detail.push_back(path);
    sendMessage(server_fd, REQUEST_FILE, detail);

    int new_port = readInt32(server_fd);  // server will give a new socket port to transfer file
    if (new_port < 0) {
        error("Server cannot provide the requested file " + path);
        return -1;
    }
    //TODO: consider using a new thread


    // connect to the new port
    struct sockaddr_in sa = this->_server_addr;
    sa.sin_port = htons(new_port);

    int recv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (recv_fd < 0) {
        error("[NetworkAgent::requestFile] Cannot open socket");
        return -1;
    }
    if (connect(recv_fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
        error("[NetworkAgent::requestFile] Cannot connect");
        return -1;
    }
    printf("Connection established\n");

    int32_t fileSize = readInt32(recv_fd);  // get file size
    printf("[NetworkAgent::requestFile] filesize=%d Bytes\n", fileSize);

    const int BUF_SZ = 8 * 1024 * 1024;
    char *buf = new char[BUF_SZ]; // 8MB
    int n;

    // tmp_file_path = tmp_dir + "/" + filename_hash
    std::string tmp_file_path = get_tmp_dir() + std::string("/") + std::to_string(std::hash<std::string>{}(path));
    FILE *tmp_fp = fopen(tmp_file_path.c_str(), "wb");
    if (tmp_fp == nullptr) {
        error("cannot open tmp file " + tmp_file_path);
        error("errno=" + std::to_string(errno));
        return -1;
    }
    while (fileSize > 0) {
        int sz = fileSize;
        if (sz > BUF_SZ)sz = BUF_SZ;
        n = recv(recv_fd, buf, sz, 0);
        if (n == 0) {
            error("read 0 byte error");
            return -1;
        }

        int n_write = fwrite(buf, n, 1, tmp_fp); // write to tmp file
        if (n_write != 1)error("Write error");
        fileSize -= n;
        printf("[debug] receive %d bytes of file, remain %d bytes\n", n, fileSize);
    }
    fclose(tmp_fp);
    printf("[debug] Transfer complete\n");

    delete[] buf;
    close(recv_fd);
    printf("[debug] tmp file write complete\n");
    CONVERT_PATH(real_path, path.c_str());
    rename(tmp_file_path.c_str(), real_path);   // TODO: check failure
    printf("[debug] rename tmp file complete\n");

    return 0;
}

void NetworkAgent::sendRequestedFile(int fd, std::string path) {
    // TODO: fill this function
    // use a new thread, open a new socket port,
    // use the main port to send the new port
    // accept connection
    // lock the file
    // send the file to client

    struct sockaddr_in sa{};
    sa.sin_family = AF_INET;
//    inet_pton(AF_INET,address,&(sa.sin_addr));
    sa.sin_addr.s_addr = INADDR_ANY;
    int32_t port = listenPort + 8; // just use any empty port
    sa.sin_port = htons(port);
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        error("[NetworkAgent::sendRequestedFile] Cannot open socket");
        return;
    }
    if (bind(socket_fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
        std::cerr << socket_fd << " "
                  //<<server_addr.sin_addr<<" "
                  << sa.sin_family << " "
                  << sa.sin_port << " "
                  << sa.sin_zero
                  << std::endl;
        error("Cannot bind socket");
        return;
    }

    listen(socket_fd, 8);
    sendRaw(fd, (char *) &port, sizeof(int32_t));

    struct sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    int send_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &client_len);
    printf("[NetworkAgent::sendRequestedFile] Connection good\n");
    if (send_fd < 0)
        error("Error on accept");
    CONVERT_PATH(real_path, path.c_str())
    int data_fd = open(real_path, O_RDONLY);
    if (data_fd == -1) {
        error("Cannot open the file to send");
        return;
    }
    struct stat file_stat{};
    if (fstat(data_fd, &file_stat) < 0) {
        error("Error fstat");
        return;
    }

    int32_t file_sz = file_stat.st_size;
    printf("[NetworkAgent::sendRequestedFile] The file size is %d Bytes\n", file_sz);

    sendRaw(send_fd, (char *) &file_sz, sizeof(int32_t)); // send file size
    int bytes_sent = 0;
    while (bytes_sent < file_sz) {
        char buf[2 * 1024 * 1024];  //2M
        int n_read = read(data_fd, buf, 2 * 1024 * 1024);
        int n_sent = 0;
        while (n_sent < n_read) {
            int sz = send(send_fd, buf + n_sent, n_read, 0);
            if (sz < 0) { error("send file error"); }
            n_sent += sz;
        }
        bytes_sent += n_sent;
    }
    printf("[debug] sent %d bytes in total\n", bytes_sent);

    close(send_fd);
    close(socket_fd);
    printf("[debug] Transfer complete\n");

}

void NetworkAgent::freeSocket() {
    for (int c: connections) {
        close(c);
    }
}
