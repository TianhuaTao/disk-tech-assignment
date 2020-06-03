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
#include <filesystem>
#include "DriveAgent.h"
#include "op.h"
#include "zerodrive_common.h"
#include "DriveServerAgent.h"
#include "DriveClientAgent.h"

using namespace ZeroDrive;

bool NetworkAgent::isConnected() {
    return !connections.empty();
}

int NetworkAgent::sendMessage(int socket_fd, enum Operation_t msg, const std::vector<std::string> &detail) {
//    printf("Send message to %d\n", socket_fd);
    int32_t msg_i = msg;
    int32_t string_cnt = detail.size();
    sendRaw(socket_fd, (const char *) &msg_i, sizeof(msg_i));
    sendRaw(socket_fd, (const char *) &string_cnt, sizeof(string_cnt));
    for (const auto &i : detail) {
        int32_t length = i.size();
        sendRaw(socket_fd, (const char *) &length, sizeof(length));
//        std::cout << i.c_str() << std::endl;   // debug
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

int NetworkAgent::listenAsync(const char *address, int port, std::function<void()> callback) {
    if (listeningThread) {
        error("NetworkAgent is already listening");
    }

    this->listenPort = port;
    struct sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(port);

    std::cerr << "[NetworkAgent::listenAsync] port = " << port << std::endl;

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

        // start message loop in a new thread
        messageLoopThreadPool.push_back(new std::thread(&NetworkAgent::MessageLoop, this, newsockfd));
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
//    MessageLoop(socket_fd);
    return 0;
}

NetworkAgent::Role NetworkAgent::getRole() const {
    return role;
}

void NetworkAgent::setRole(NetworkAgent::Role newRole) {
    NetworkAgent::role = newRole;
}


void NetworkAgent::MessageLoop(int sockfd) {
    printf("Start message loop with socket_fd %d\n", sockfd);
    while (!shutDown) {
        // read message token
        enum Operation_t token = readToken(sockfd);
        if (token == PUSH) {
            std::cout << "Receive: PUSH\n";
            std::vector<std::string> newFiles;
            std::vector<std::string> deleteFiles;
            std::vector<std::string> newDirs;
            std::vector<std::string> deleteDirs;
            std::vector<std::pair<std::string, std::string>> renameDirs;

            int32_t string_cnt;
            token = readToken(sockfd);
            // token == MKDIR
            string_cnt = readInt32(sockfd);
            std::cout << string_cnt << " new dirs\n";
            for (int i = 0; i < string_cnt; ++i) {
                auto path = readString(sockfd);
                std::cout << "mkdir=" << path << std::endl;
                newDirs.push_back(path);
            }

            token = readToken(sockfd);
            // token == RMDIR
            string_cnt = readInt32(sockfd);
            std::cout << string_cnt << " delete dirs\n";
            for (int i = 0; i < string_cnt; ++i) {
                auto path = readString(sockfd);
                std::cout << "rmdir=" << path << std::endl;
                deleteDirs.push_back(path);
            }

            token = readToken(sockfd);
            // token == UPDATE
            string_cnt = readInt32(sockfd);
            std::cout << string_cnt << " updates\n";
            for (int i = 0; i < string_cnt; ++i) {
                auto path = readString(sockfd);
                std::cout << "update=" << path << std::endl;
                newFiles.push_back(path);
            }

            token = readToken(sockfd);
            // token == DELETE
            string_cnt = readInt32(sockfd);
            std::cout << string_cnt << " deletes\n";
            for (int i = 0; i < string_cnt; ++i) {
                auto path = readString(sockfd);
                std::cout << "delete=" << path << std::endl;
                deleteFiles.push_back(path);
            }

            token = readToken(sockfd);
            // token == RENAME_DIR
            string_cnt = readInt32(sockfd);
            std::cout << string_cnt / 2 << " dir renames\n";
            for (int i = 0; i < string_cnt / 2; ++i) {
                auto from = readString(sockfd);
                auto to = readString(sockfd);
                std::cout << "from=" << from << ", to=" << to << std::endl;
                renameDirs.emplace_back(from, to);
            }
            dynamic_cast<DriveServerAgent *>(hostDriveAgent)->handleUpdate(sockfd,
                                                                           newFiles, deleteFiles, newDirs,
                                                                           deleteDirs, renameDirs);


            // token == PULL
        } else if (token == PULL) {
            std::cout << "Receive: PULL, ";

            uint64_t last_sync = readUint64(sockfd);
            std::cout << "last_sync=" << last_sync << "\n";

            dynamic_cast<DriveServerAgent *>(hostDriveAgent)
                    ->handlePull(sockfd, last_sync);
        } else {
            std::cout << "Can't recognize the token..." << token << std::endl;
            break;
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


void NetworkAgent::freeSocket() {
    for (int c: connections) {
        close(c);
    }
}

enum Operation_t NetworkAgent::readToken(int sockfd) {
    int32_t token_i;
    int ret = readBytes(sockfd, (char *) &token_i, sizeof(int32_t));
    if (ret < 0) error("readToken error");
    auto token = (Operation_t) token_i;
    return token;
}

uint64_t NetworkAgent::pushToServer(std::set<std::string> &newFiles,
                                    std::set<std::string> &deleteFiles,
                                    std::set<std::string> &newDirs,
                                    std::set<std::string> &deleteDirs,
                                    std::set<std::pair<std::string, std::string>> &renameDirs) {
//    printf("debug 0\n");
    int server_fd = *connections.begin();
    std::vector<std::string> newF(newFiles.begin(), newFiles.end());
    std::vector<std::string> delF(deleteFiles.begin(), deleteFiles.end());
    std::vector<std::string> newD(newDirs.begin(), newDirs.end());
    std::vector<std::string> delD(deleteDirs.begin(), deleteDirs.end());
    std::vector<std::string> renameD;
    for (const auto &p:renameDirs) {
        renameD.push_back(p.first);
        renameD.push_back(p.second);
    }


    Operation_t token = PUSH;
    sendRaw(server_fd, (char *) &token, sizeof(token));


    sendMessage(server_fd, MKDIR, newD);
    sendMessage(server_fd, RMDIR, delD);
    sendMessage(server_fd, UPDATE, newF);
    sendMessage(server_fd, DELETE, delF);
    sendMessage(server_fd, RENAME_DIR, renameD);

//    printf("Message sent\n");
    for (const auto &path: newF) {
        uploadFile(server_fd, path);
    }

    uint64_t newStamp = readUint64(server_fd);
    return newStamp;
}

void NetworkAgent::uploadFile(int remote_fd, const std::string &path) {
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
    printf("[NetworkAgent::uploadFile] The file size is %d Bytes: %s\n", file_sz, path.c_str());

    sendRaw(remote_fd, (char *) &file_sz, sizeof(int32_t)); // send file size
    int bytes_sent = 0;
    while (bytes_sent < file_sz) {
        char buf[2 * 1024 * 1024];  //2M
        int n_read = read(data_fd, buf, 2 * 1024 * 1024);
        int n_sent = 0;
        while (n_sent < n_read) {
            int sz = send(remote_fd, buf + n_sent, n_read, 0);
            if (sz < 0) { error("send file error"); }
            n_sent += sz;
        }
        bytes_sent += n_sent;
    }
    printf("[NetworkAgent::uploadFile] Transfer complete. Sent %d bytes in total\n", bytes_sent);

}

void NetworkAgent::downloadFile(int remote_fd, const std::string &path) {
    int32_t fileSize = readInt32(remote_fd);  // get file size
    printf("[NetworkAgent::downloadFile] %s, filesize=%d Bytes\n", path.c_str(), fileSize);

    const int BUF_SZ = 8 * 1024 * 1024;
    char *buf = new char[BUF_SZ]; // 8MB
    int n;

    // tmp_file_path = tmp_dir + "/" + filename_hash
    std::string tmp_file_path = get_tmp_dir() + std::string("/") + std::to_string(std::hash<std::string>{}(path));
    FILE *tmp_fp = fopen(tmp_file_path.c_str(), "wb");
    if (tmp_fp == nullptr) {
        error("cannot open tmp file " + tmp_file_path);
        error("errno=" + std::to_string(errno));
        return;
    }
    while (fileSize > 0) {
        int sz = fileSize;
        if (sz > BUF_SZ)sz = BUF_SZ;
        n = recv(remote_fd, buf, sz, 0);
        if (n == 0) {
            error("read 0 byte error");
            return;
        }

        int n_write = fwrite(buf, n, 1, tmp_fp); // write to tmp file
        if (n_write != 1)error("Write error");
        fileSize -= n;
        printf("[NetworkAgent::downloadFile] receive %d bytes of file, remain %d bytes\n", n, fileSize);
    }
    fclose(tmp_fp);
    printf("[NetworkAgent::downloadFile] Transfer complete\n");

    delete[] buf;
//    close(remote_fd);
    printf("[NetworkAgent::downloadFile] tmp file write complete: %s\n", tmp_file_path.c_str());
    CONVERT_PATH(real_path, path.c_str());
    rename(tmp_file_path.c_str(), real_path);   // TODO: check failure
    printf("[NetworkAgent::downloadFile] rename tmp file complete: %s\n", real_path);

}

uint64_t NetworkAgent::pullfromServer(uint64_t last_sync) {
    int server_fd = *connections.begin();
    Operation_t token = PULL;
    sendRaw(server_fd, (char *) &token, sizeof(token));  // send token
    sendRaw(server_fd, (char *) &last_sync, sizeof(last_sync)); // send last_sync
    token = readToken(server_fd);
    // receive
    if (token == DOWNLOAD_ALL) {
        /// corresponding to NetworkAgent::sendAllData
        printf("[Pull] Ready to download everything\n");
        int32_t string_cnt;
        (void) string_cnt;
        while (true) {
            token = readToken(server_fd);
            printf("[pullfromServer] token=%d\n", token);

            if (token == NONE) {
                printf("[pullfromServer] token=NONE\n");
                break;
            }
            if (token == MKDIR) {
                printf("[pullfromServer] token=MKDIR\n");

                string_cnt = readInt32(server_fd);  // should be 1

                if (string_cnt != 1) {
                    error("Value error");
                }
                auto path = readString(server_fd);

                std::cout << "mkdir=" << path;
                CONVERT_PATH(real_path, path.c_str())
                if (mkdir(real_path, 0777) == 0)
                    std::cout << " [successful]" << std::endl;
                else
                    std::cout << " [failed]" << std::endl;
            } else if (token == OPEN) {
                printf("[pullfromServer] token=OPEN\n");

                string_cnt = readInt32(server_fd);  // should be 1
                if (string_cnt != 1) error("Value error");

                auto path = readString(server_fd);
                std::cout << "download=" << path;

                downloadFile(server_fd, path);
            } else {
                error("Unknown token");
                break;
            }
        }
        auto stamp = readUint64(server_fd);

        printf("[Pull] All done, new stamp = %lu\n", stamp);
        return stamp;

    } else if (token == PATCH) {
        printf("[Pull] Ready to download updated files\n");
        /// copied form MessageLoop
        std::vector<std::string> newFiles;
        std::vector<std::string> deleteFiles;
        std::vector<std::string> newDirs;
        std::vector<std::string> deleteDirs;
        std::vector<std::pair<std::string, std::string>> renameDirs;

        int32_t string_cnt;
        token = readToken(server_fd);
        // token == MKDIR
        string_cnt = readInt32(server_fd);
        std::cout << string_cnt << " new dirs\n";
        for (int i = 0; i < string_cnt; ++i) {
            auto path = readString(server_fd);
            std::cout << "mkdir=" << path << std::endl;
            newDirs.push_back(path);
        }

        token = readToken(server_fd);
        // token == RMDIR
        string_cnt = readInt32(server_fd);
        std::cout << string_cnt << " delete dirs\n";
        for (int i = 0; i < string_cnt; ++i) {
            auto path = readString(server_fd);
            std::cout << "rmdir=" << path << std::endl;
            deleteDirs.push_back(path);
        }
        token = readToken(server_fd);
        // token == UPDATE
        string_cnt = readInt32(server_fd);
        std::cout << string_cnt << " updates\n";
        for (int i = 0; i < string_cnt; ++i) {
            auto path = readString(server_fd);
            std::cout << "update=" << path << std::endl;
            newFiles.push_back(path);
        }

        token = readToken(server_fd);
        // token == DELETE
        string_cnt = readInt32(server_fd);
        std::cout << string_cnt << " deletes\n";
        for (int i = 0; i < string_cnt; ++i) {
            auto path = readString(server_fd);
            std::cout << "delete=" << path << std::endl;
            deleteFiles.push_back(path);
        }

        token = readToken(server_fd);
        // token == RENAME_DIR
        string_cnt = readInt32(server_fd);
        std::cout << string_cnt / 2 << " dir renames\n";
        for (int i = 0; i < string_cnt / 2; ++i) {
            auto from = readString(server_fd);
            auto to = readString(server_fd);
            std::cout << "from=" << from << ", to=" << to << std::endl;
            renameDirs.emplace_back(from, to);
        }
        dynamic_cast<DriveClientAgent *>(hostDriveAgent)->handleUpdate(server_fd,
                                                                       newFiles, deleteFiles, newDirs,
                                                                       deleteDirs, renameDirs);
        uint64_t stamp = readUint64(server_fd);

        return stamp;

    } else if (token == UP_TO_DATE) {
        printf("[Pull] Zerodrive is up to date\n");
        return last_sync;
    } else {
        printf("[Pull] Unknown token %d\n", token);
        return last_sync;
    }
}

uint64_t NetworkAgent::readUint64(int fd) {
    uint64_t ret;
    readBytes(fd, (char *) &ret, sizeof(ret));
    return ret;
}

/// corresponding to NetworkAgent::pullfromServer
void NetworkAgent::sendAllData(int fd) {
    printf("[NetworkAgent::sendAllData]\n");
    hostDriveAgent->lock();
    auto prefix = std::string(ZeroDrive::get_data_dir());
    printf("[NetworkAgent::sendAllData] %s\n", ZeroDrive::get_data_dir());
    for (const auto &dirEntry: std::filesystem::recursive_directory_iterator(ZeroDrive::get_data_dir())) {

        if (dirEntry.is_directory()) {
            std::vector<std::string> args;
            auto fullPath = dirEntry.path().string();
            std::cout << "[sendAllData] " << fullPath << "\n";

            auto relativePath = fullPath.erase(0, prefix.length());
            args.push_back(relativePath);
            std::cout << "[sendAllData] mkdir=" << relativePath << "\n";

            sendMessage(fd, MKDIR, args);
        } else if (dirEntry.is_regular_file()) {
            std::vector<std::string> args;
            auto fullPath = dirEntry.path().string();
            auto relativePath = fullPath.erase(0, prefix.length());
            args.push_back(relativePath);

            std::cout << "[sendAllData] out=" << relativePath << "\n";
            sendMessage(fd, OPEN, args);
            uploadFile(fd, fullPath);
        } else {
            std::cout << "[NetworkAgent::sendAllData] special file: " << dirEntry << "\n";
        }
    }
    sendToken(fd, NONE);
    auto stamp = dynamic_cast<DriveServerAgent *>(hostDriveAgent)->getServerStamp();
    sendRaw(fd, (char *) &stamp, sizeof(stamp));
}

void
NetworkAgent::pushToClient(int fd, const std::set<std::string> &newFiles,
                           const std::set<std::string> &deleteFiles,
                           const std::set<std::string> &newDirs,
                           const std::set<std::string> &deleteDirs,
                           const std::set<std::pair<std::string, std::string>> &renameDirs) {

    std::vector<std::string> newF(newFiles.begin(), newFiles.end());
    std::vector<std::string> delF(deleteFiles.begin(), deleteFiles.end());
    std::vector<std::string> newD(newDirs.begin(), newDirs.end());
    std::vector<std::string> delD(deleteDirs.begin(), deleteDirs.end());
    std::vector<std::string> renameD;
    for (const auto &p:renameDirs) {
        renameD.push_back(p.first);
        renameD.push_back(p.second);
    }


//    Operation_t token = PUSH;
//    sendRaw(fd, (char *) &token, sizeof(token));


    sendMessage(fd, MKDIR, newD);
    sendMessage(fd, RMDIR, delD);
    sendMessage(fd, UPDATE, newF);
    sendMessage(fd, DELETE, delF);
    sendMessage(fd, RENAME_DIR, renameD);

//    printf("Message sent\n");
    for (const auto &path: newF) {
        uploadFile(fd, path);   // send to client
    }
}

int NetworkAgent::sendToken(int socket_fd, Operation_t token) {
    int32_t i = token;
    return sendRaw(socket_fd, (char *) &i, sizeof(i));
}
