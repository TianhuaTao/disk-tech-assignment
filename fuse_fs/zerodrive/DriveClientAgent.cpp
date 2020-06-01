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
    networkAgent = new NetworkAgent(this);
    networkAgent->setRole(NetworkAgent::CLIENT);
    networkAgent->connectAsync(address, port, []() {
        std::cout << "Got connection\n";
    });
    fileOperation = new FileOperation();
    printf("NetworkAgent init complete\n");
}

DriveClientAgent::~DriveClientAgent() {
    delete networkAgent;
    delete fileOperation;
    printf("Server agent shut down\n");
}

void *DriveClientAgent::Init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
    return fileOperation->Init(conn,cfg);
}

//-------------------init-------------------------------------------------------

int DriveClientAgent::Read(const char *path, char *buf, size_t size, 
                            off_t offset, struct fuse_file_info *fi) {
    return fileOperation->Read(path, buf, size, offset, fi);
    // send signal to client
    //std::vector<std::string> detail;
    //detail.push_back(std::string(path));
    //this->broadcastChanges(WRITE_DONE, detail);
}

int DriveClientAgent::Getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    return fileOperation->Getattr(path,stbuf,fi);
    //TODO: send signal to client
    // send signal to client
    //std::vector<std::string> detail;
    //detail.push_back(std::string(path));
    //this->broadcastChanges(WRITE_DONE, detail);
}

//---------------------will do something-----------------------------------------------------------------

int DriveClientAgent::Write(const char *path, const char *buf, size_t size, 
                        off_t offset, struct fuse_file_info *fi) {
    // send signal to client
    std::vector<std::string> detail;
    detail.push_back(std::string(path));
    this->broadcastChanges(WRITE_DONE, detail);
    return fileOperation->Write(path, buf, size, offset, fi);
}

int DriveClientAgent::Rename(const char *from, const char *to, unsigned int flags) {
    // send signal to server
    std::vector<std::string> detail;
    detail.push_back(std::string(from));
    detail.push_back(std::string(to));
    this->broadcastChanges(RENAME, detail);
    return fileOperation->Rename(from, to, flags);

}

int DriveClientAgent::Open(const char *path, struct fuse_file_info *fi) {
    return fileOperation->Open(path,fi);
    //TODO: send signal to client
}

int DriveClientAgent::Readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi,
                          enum fuse_readdir_flags flags) {
    return fileOperation->Readdir(path,buf,filler, offset,fi,flags);

    //TODO: before I read the directory, I should keep the info updated
}

int DriveClientAgent::Create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    // send signal to server
    std::vector<std::string> detail;
    detail.push_back(std::string(path));
    detail.push_back(std::to_string(mode));
    this->broadcastChanges(CREATE, detail);
    return fileOperation->Create(path,mode,fi);
}

int DriveClientAgent::Mkdir(const char *path, mode_t mode) {
    // TODO: send signal to client
    // send signal to client
    std::vector<std::string> detail;
    detail.push_back(std::string(path));
    detail.push_back(std::to_string(mode));
    this->broadcastChanges(MKDIR, detail);

    return fileOperation->Mkdir(path, mode);
}

int DriveClientAgent::Rmdir(const char *path) {
    // send signal to server
    // TODO: 
    std::vector<std::string> detail;
    detail.push_back(std::string(path));
    this->broadcastChanges(RMDIR, detail);
    return fileOperation->Rmdir(path);
}

int DriveClientAgent::Symlink(const char *from, const char *to) {
    return fileOperation->Symlink(from, to);
    // TODO: send signal to client
    //what will symlink do?????????
    //std::vector<std::string> detail;
    //detail.push_back(std::string(from));
    //detail.push_back(std::string(to));
    //this->broadcastChanges(WRITE_DONE, detail);
}

int DriveClientAgent::Chmod(const char *path, mode_t mode, struct fuse_file_info *fi) {

    // TODO: send signal to client
    // send signal to server
    std::vector<std::string> detail;
    detail.push_back(std::string(path));
    detail.push_back(std::to_string(mode));
    this->broadcastChanges(CHMOD, detail);

    return fileOperation->Chmod(path, mode, fi);
}

int DriveClientAgent::Chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi) {
    // TODO: send signal to client
    // send signal to server
    std::vector<std::string> detail;
    detail.push_back(std::string(path));
    detail.push_back(std::to_string(uid));
    detail.push_back(std::to_string(gid));
    this->broadcastChanges(CHOWN, detail);

    return fileOperation->Chown(path, uid, gid, fi);
}

int DriveClientAgent::Readlink(const char *path, char *buf, size_t size) {
    return fileOperation->Readlink(path, buf, size);


}

int DriveClientAgent::broadcastChanges(enum Message msg, std::vector<std::string> detail) {
    if(msg>0&&msg<10){
        assert(detail.size() == (unsigned long) Message_Number[msg]);
        networkAgent->sendMessageToAll(msg, detail);
    }else
    {
        std::cout << "Message not supported\n";
    }
    return 0;
}

void DriveClientAgent::onMsgWriteDone(std::string path) {
//    DriveAgent::onMsgWriteDone(path);
    printf("[DriveClientAgent::onMsgWriteDone]\n");
    // TODO: use another thread
    networkAgent->requestFile(path);    // terrible -- will take a long time

}

void DriveClientAgent::onMsgCreate(std::string path, mode_t mode) {
    DriveAgent::onMsgCreate(path, mode);
}

void DriveClientAgent::onMsgMkdir(std::string path, mode_t mode) {
    DriveAgent::onMsgMkdir(path, mode);
}

void DriveClientAgent::onMsgRename(std::string from, std::string to) {
    DriveAgent::onMsgRename(from, to);
}

void DriveClientAgent::onMsgRmdir(std::string path) {
    DriveAgent::onMsgRmdir(path);
}

void DriveClientAgent::onMsgChmod(std::string path, mode_t mode) {
    DriveAgent::onMsgChmod(path, mode);
}
