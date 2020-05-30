#define FUSE_USE_VERSION 31

#include "DriveAgent.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <cassert>
#include <dirent.h>
#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <cstdio>
#include <pwd.h>
#include <unistd.h>
#include "NetworkAgent.h"
#include <iostream>
#include "op.h"
#include "Protocol.h"
#include "DriveClientAgent.h"

DriveClientAgent::DriveClientAgent(const char *address, int port) {
    networkAgent= new NetworkAgent();
    networkAgent->setRole(NetworkAgent::CLIENT);
    networkAgent->connectAsync(address, port, []() {
        std::cout << "Callback: Got connection\n";
    });

    printf("NetworkAgent init complete\n");
    // TODO: connect to server
}

int DriveClientAgent::Rename(const char *from, const char *to, unsigned int flags) {
    // TODO: implement this function
    return 0;
}

int DriveClientAgent::Open(const char *path, struct fuse_file_info *fi) {
    // TODO: implement this function
    return 0;
}

int DriveClientAgent::Read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    // TODO: implement this function
    return 0;
}

int DriveClientAgent::Write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    // TODO: implement this function
    return 0;
}

int
DriveClientAgent::Readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi,
                          enum fuse_readdir_flags flags) {
    // TODO: implement this function
    return 0;
}

int DriveClientAgent::Create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    // TODO: implement this function
    return 0;
}

int DriveClientAgent::Getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    // TODO: implement this function
    return 0;
}

DriveClientAgent::~DriveClientAgent() {
    printf("Client agent shut down\n");
}

void *DriveClientAgent::Init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
    // TODO: implement this function
    return nullptr;
}

int DriveClientAgent::Mkdir(const char *path, mode_t mode) {
    // TODO: implement this function
    return 0;
}

int DriveClientAgent::Rmdir(const char *path) {
    // TODO: implement this function
    return 0;
}

int DriveClientAgent::Symlink(const char *from, const char *to) {
    // TODO: implement this function
    return 0;
}

int DriveClientAgent::Chmod(const char *path, mode_t mode, struct fuse_file_info *fi) {
    // TODO: implement this function
    return 0;
}

int DriveClientAgent::Chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi) {
    // TODO: implement this function
    return 0;
}

int DriveClientAgent::Readlink(const char *path, char *buf, size_t size) {
    // TODO: implement this function
    return 0;
}
