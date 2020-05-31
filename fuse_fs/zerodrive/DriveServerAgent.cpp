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

#include "DriveServerAgent.h"

DriveServerAgent::DriveServerAgent(const char *address, int port) {
    networkAgent = new NetworkAgent();
    networkAgent->setRole(NetworkAgent::SERVER);
    networkAgent->listenAsync(address, port, []() {
        std::cout << "Got connection\n";
    });
    fileOperation = new FileOperation();
    printf("NetworkAgent init complete\n");
}

DriveServerAgent::~DriveServerAgent() {
    delete networkAgent;
    delete fileOperation;
    printf("Server agent shut down\n");
}

void *DriveServerAgent::Init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
    return fileOperation->Init(conn,cfg);
    /*
    (void) conn;
    cfg->use_ino = 1;

    /// Pick up changes from lower filesystem right away. This is
	   also necessary for better hardlink support. When the kernel
	   calls the unlink() handler, it does not know the inode of
	   the to-be-removed entry and can therefore not invalidate
	   the cache of the associated inode - resulting in an
	   incorrect st_nlink value being reported for any remaining
	   hardlinks to this inode. //
    cfg->entry_timeout = 0;
    cfg->attr_timeout = 0;
    cfg->negative_timeout = 0;

    printf("data path: %s\n", get_data_dir());

    // TODO: check failure
    if (mkdir(get_data_dir(), 0777) == -1) {
        if (errno == EEXIST) {
            // already exists
            printf("found previous data\n");
        } else {
            printf("cannot create data folder\n");
        }
    }

    return nullptr;
    */
}

//-------------------init-------------------------------------------------------

int DriveServerAgent::Read(const char *path, char *buf, size_t size, 
                            off_t offset, struct fuse_file_info *fi) {
    return fileOperation->Read(path, buf, size, offset, fi);
    // send signal to client
    //std::vector<std::string> detail;
    //detail.push_back(std::string(path));
    //this->broadcastChanges(WRITE_DONE, detail);
}

int DriveServerAgent::Getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    return fileOperation->Getattr(path,stbuf,fi);
    //TODO: send signal to client
    // send signal to client
    //std::vector<std::string> detail;
    //detail.push_back(std::string(path));
    //this->broadcastChanges(WRITE_DONE, detail);
}

//---------------------will do something-----------------------------------------------------------------

int DriveServerAgent::Write(const char *path, const char *buf, size_t size, 
                        off_t offset, struct fuse_file_info *fi) {
    // send signal to client
    std::vector<std::string> detail;
    detail.push_back(std::string(path));
    this->broadcastChanges(WRITE_DONE, detail);
    return fileOperation->Write(path, buf, size, offset, fi);
}

int DriveServerAgent::Rename(const char *from, const char *to, unsigned int flags) {
    // TODO: send signal to client
    // send signal to client
    std::vector<std::string> detail;
    detail.push_back(std::string(from));
    detail.push_back(std::string(to));
    this->broadcastChanges(RENAME, detail);
    return fileOperation->Rename(from, to, flags);
}

int DriveServerAgent::Open(const char *path, struct fuse_file_info *fi) {
    return fileOperation->Open(path,fi);

    //TODO: send signal to client
    // send signal to client
    //std::vector<std::string> detail;
    //detail.push_back(std::string(path));
    //this->broadcastChanges(WRITE_DONE, detail);
}

int DriveServerAgent::Readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi,
                          enum fuse_readdir_flags flags) {
    return fileOperation->Readdir(path,buf,filler, offset,fi,flags);

    //TODO: before I read the directory, I should keep the info updated
    //std::vector<std::string> detail;
    //detail.push_back(std::string(path));
    //this->broadcastChanges(WRITE_DONE, detail);
}

int DriveServerAgent::Create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    // TODO: send signal to client
    // send signal to client
    std::vector<std::string> detail;
    detail.push_back(std::string(path));
    detail.push_back(std::to_string(mode));
    this->broadcastChanges(CREATE, detail);
    return fileOperation->Create(path,mode,fi);
}

int DriveServerAgent::Mkdir(const char *path, mode_t mode) {
    // TODO: send signal to client
    // send signal to client
    std::vector<std::string> detail;
    detail.push_back(std::string(path));
    detail.push_back(std::to_string(mode));
    this->broadcastChanges(MKDIR, detail);

    return fileOperation->Mkdir(path, mode);
}

int DriveServerAgent::Rmdir(const char *path) {
    // TODO: send signal to client
    // send signal to client
    std::vector<std::string> detail;
    detail.push_back(std::string(path));
    this->broadcastChanges(RMDIR, detail);
    return fileOperation->Rmdir(path);
}

int DriveServerAgent::Symlink(const char *from, const char *to) {
    return fileOperation->Symlink(from, to);
    // TODO: send signal to client
    //what will symlink do?????????
    //std::vector<std::string> detail;
    //detail.push_back(std::string(from));
    //detail.push_back(std::string(to));
    //this->broadcastChanges(WRITE_DONE, detail);
}

int DriveServerAgent::Chmod(const char *path, mode_t mode, struct fuse_file_info *fi) {

    // TODO: send signal to client
    // send signal to client
    std::vector<std::string> detail;
    detail.push_back(std::string(path));
    detail.push_back(std::to_string(mode));
    this->broadcastChanges(CHMOD, detail);

    return fileOperation->Chmod(path, mode, fi);
}

int DriveServerAgent::Chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi) {
    // TODO: send signal to client
    // send signal to client
    std::vector<std::string> detail;
    detail.push_back(std::string(path));
    detail.push_back(std::to_string(uid));
    detail.push_back(std::to_string(gid));
    this->broadcastChanges(CHOWN, detail);

    return fileOperation->Chown(path, uid, gid, fi);
}

int DriveServerAgent::Readlink(const char *path, char *buf, size_t size) {
    return fileOperation->Readlink(path, buf, size);

    // send signal to client
    //????????????but what will this operation do???
    return 0;
}

int DriveServerAgent::broadcastChanges(enum Message msg, std::vector<std::string> detail) {
    switch (msg) {
        // TODO: complete code
        case NONE:
            break;
        case WRITE_DONE:
            assert(detail.size() == 1);
            networkAgent->sendMessageToAll(msg, detail);
            break;
        case RENAME:
            assert(detail.size() == 2);
            networkAgent->sendMessageToAll(msg, detail);
            break;
        case CREATE:
            assert(detail.size() == 2);
            networkAgent->sendMessageToAll(msg, detail);
            break;
        case MKDIR:
            assert(detail.size() == 2);
            networkAgent->sendMessageToAll(msg, detail);
            break;
        case RMDIR:
            assert(detail.size() == 1);
            networkAgent->sendMessageToAll(msg, detail);
            break;
        case CHMOD:
            assert(detail.size() == 2);
            networkAgent->sendMessageToAll(msg, detail);
            break;
        case CHOWN:
            assert(detail.size() == 3);
            networkAgent->sendMessageToAll(msg, detail);
            break;
        default:
            std::cout << "Message not supported\n";
            break;
    }


    return 0;
}

