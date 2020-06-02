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
#include "zerodrive_common.h"

using namespace ZeroDrive;

DriveClientAgent::DriveClientAgent(const char *address, int port) {
    networkAgent = new NetworkAgent(this);
    networkAgent->setRole(NetworkAgent::CLIENT);
    networkAgent->connectAsync(address, port, []() {
        std::cout << "Got connection\n";
    });
    fileOperation = new FileOperation();
    backgroundUpdater = new BackgroundUpdater(this);
    backgroundUpdater->run();
    printf("Cliend agent init complete\n");
}

DriveClientAgent::~DriveClientAgent() {
    delete networkAgent;
    delete fileOperation;
    printf("Client agent shut down\n");
}

void *DriveClientAgent::Init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
    return fileOperation->Init(conn, cfg);
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
    return fileOperation->Getattr(path, stbuf, fi);
    //TODO: send signal to client
    // send signal to client
    //std::vector<std::string> detail;
    //detail.push_back(std::string(path));
    //this->broadcastChanges(WRITE_DONE, detail);
}

//---------------------will do something-----------------------------------------------------------------

int DriveClientAgent::Write(const char *path, const char *buf, size_t size,
                            off_t offset, struct fuse_file_info *fi) {

    int ret = fileOperation->Write(path, buf, size, offset, fi);
    std::vector<std::string> detail;
    detail.emplace_back(path);
    if (ret >= 0) {
        OperationRecord r = OperationRecord(WRITE_DONE, detail);
        backgroundUpdater->dirtyChanges.push_back(r);
    }
    return ret;
}

int DriveClientAgent::Rename(const char *from, const char *to, unsigned int flags) {
    std::vector<std::string> detail;
    detail.emplace_back(from);
    detail.emplace_back(to);
    bool isDir = fileOperation->checkIsDir(from);
    int ret = fileOperation->Rename(from, to, flags);
    if (ret >= 0) {
        OperationRecord r = OperationRecord(isDir ? RENAME_DIR : RENAME, detail);
        backgroundUpdater->dirtyChanges.push_back(r);
    }
    return ret;

}

int DriveClientAgent::Open(const char *path, struct fuse_file_info *fi) {
    bool exist_before = fileOperation->checkExist(path);
    int ret = fileOperation->Open(path, fi);
    if (!exist_before && ret >= 0) { // newly created
        OperationRecord r = OperationRecord(OPEN, path);
        backgroundUpdater->dirtyChanges.push_back(r);
    }
    return ret;

}

int
DriveClientAgent::Readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi,
                          enum fuse_readdir_flags flags) {
    return fileOperation->Readdir(path, buf, filler, offset, fi, flags);

}

int DriveClientAgent::Create(const char *path, mode_t mode, struct fuse_file_info *fi) {

    std::vector<std::string> detail;
    detail.emplace_back(path);
    detail.push_back(std::to_string(mode));
    int ret = fileOperation->Create(path, mode, fi);
    if (ret >= 0) {
        OperationRecord r = OperationRecord(CREATE, detail);
        backgroundUpdater->dirtyChanges.push_back(r);
    }
    return ret;
}

int DriveClientAgent::Mkdir(const char *path, mode_t mode) {

    int ret = fileOperation->Mkdir(path, mode);
    std::vector<std::string> detail;
    detail.emplace_back(path);
    detail.push_back(std::to_string(mode));
    if (ret >= 0) {
        OperationRecord r = OperationRecord(MKDIR, detail);
        backgroundUpdater->dirtyChanges.push_back(r);
    }
    return ret;
}

int DriveClientAgent::Rmdir(const char *path) {
    int ret = fileOperation->Rmdir(path);
    std::vector<std::string> detail;
    detail.emplace_back(path);
    if (ret >= 0) {
        OperationRecord r = OperationRecord(RMDIR, detail);
        backgroundUpdater->dirtyChanges.push_back(r);
    }
    return ret;
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
    detail.emplace_back(path);
    detail.push_back(std::to_string(mode));


    return fileOperation->Chmod(path, mode, fi);
}

int DriveClientAgent::Chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi) {
    // TODO: send signal to client
    // send signal to server
    std::vector<std::string> detail;
    detail.emplace_back(path);
    detail.push_back(std::to_string(uid));
    detail.push_back(std::to_string(gid));
//    this->broadcastChanges(CHOWN, detail);

    return fileOperation->Chown(path, uid, gid, fi);
}

int DriveClientAgent::Readlink(const char *path, char *buf, size_t size) {
    return fileOperation->Readlink(path, buf, size);


}

//int DriveClientAgent::broadcastChanges(enum Operation_t msg, std::vector<std::string> detail) {
//    if(msg>0&&msg<10){
//        assert(detail.size() == (unsigned long) Message_Number[msg]);
//        networkAgent->sendMessageToAll(msg, detail);
//    }else
//    {
//        std::cout << "Operation_t not supported\n";
//    }
//    return 0;
//}

//void DriveClientAgent::onMsgWriteDone(std::string path) {
////    DriveAgent::onMsgWriteDone(path);
//    printf("[DriveClientAgent::onMsgWriteDone]\n");
//    // TODO: use another thread
//    networkAgent->requestFile(path);    // terrible -- will take a long time
//
//}

//void DriveClientAgent::onMsgCreate(std::string path, mode_t mode) {
//    DriveAgent::onMsgCreate(path, mode);
//}
//
//void DriveClientAgent::onMsgMkdir(std::string path, mode_t mode) {
//    DriveAgent::onMsgMkdir(path, mode);
//}
//
//void DriveClientAgent::onMsgRename(std::string from, std::string to) {
//    DriveAgent::onMsgRename(from, to);
//}
//
//void DriveClientAgent::onMsgRmdir(std::string path) {
//    DriveAgent::onMsgRmdir(path);
//}
//
//void DriveClientAgent::onMsgChmod(std::string path, mode_t mode) {
//    DriveAgent::onMsgChmod(path, mode);
//}

int DriveClientAgent::Unlink(const char *path) {
    int ret = fileOperation->Unlink(path);
    std::vector<std::string> detail;
    detail.emplace_back(path);
    if (ret >= 0) {
        OperationRecord r = OperationRecord(UNLINK, detail);
        backgroundUpdater->dirtyChanges.push_back(r);
    }
    return ret;
}

void DriveClientAgent::handleUpdate(int connection_fd, const std::vector<std::string> &newFiles,
                                    const std::vector<std::string> &deleteFiles,
                                    const std::vector<std::string> &newDirs,
                                    const std::vector<std::string> &deleteDirs,
                                    const std::vector<std::pair<std::string, std::string>> &renameDirs) {
    printf("[DriveClientAgent] Processing updates\n");
    // TODO: this is buggy, just like DriveServerAgent
    for (const auto &d: deleteDirs) {
//        fileOperation->Rmdir(d.c_str() );
        this->Rmdir(d.c_str());
    }
    for (const auto &path: deleteFiles) {
//        fileOperation->Unlink(path.c_str());
        this->Unlink(path.c_str());
    }
    for (const auto &d: newDirs) {
//        fileOperation->Mkdir(d.c_str(), 0777);
        this->Mkdir(d.c_str(), 0777);
    }
    for (const auto &path: newFiles) {
        networkAgent->downloadFile(connection_fd, path);
    }
    for (const auto &p: renameDirs) {
//       fileOperation->Rename(p.first.c_str(),p.second.c_str(), 0);
        this->Rename(p.first.c_str(), p.second.c_str(), 0);
    }
}

void DriveClientAgent::BackgroundUpdater::run() {
    running = true;

    updatingThread = new std::thread([&]() {
        while (this->running) {
            update();
            ZeroDrive::sleep_seconds(5);
        }
    });
}

DriveClientAgent::BackgroundUpdater::BackgroundUpdater(DriveClientAgent *driveClientAgent)
        : host(driveClientAgent) {
}


void DriveClientAgent::BackgroundUpdater::update() {
    if (!host->networkAgent->isConnected()) return;

    auto stamp = host->networkAgent->pullfromServer(host->last_sync);
    host->last_sync = stamp;

    printf("update %d local changes\n", dirtyChanges.size());
    std::set<std::string> newFiles;     // the files that are modified
    std::set<std::string> deleteFiles;
    std::set<std::string> newDirs;
    std::set<std::string> deleteDirs;
    std::set<std::pair<std::string, std::string>> renameDirs;

    while (dirtyChanges.size() > 0) {
        // merge changes
//        printf("dddd\n");

        auto c = dirtyChanges.front();
        dirtyChanges.pop_front();
        auto path = c.args[0];
        auto op = c.type;
//        std::cout<< path<<"\n";
        if (op == CREATE || op == WRITE_DONE || op == OPEN) {
            newFiles.insert(path);
            deleteFiles.erase(path);
        } else if (op == RENAME) {
            newFiles.insert(c.args[1]);
            deleteFiles.erase(c.args[1]);
            newFiles.erase(path);
            deleteFiles.insert(path);
        } else if (op == UNLINK) {
            deleteFiles.insert(path);
            newFiles.erase(path);
        } else if (op == MKDIR) {
            deleteDirs.erase(path);
            newDirs.insert(path);
        } else if (op == RMDIR) {
            deleteDirs.insert(path);
            newDirs.erase(path);
        } else if (op == RENAME_DIR) {
            auto to = c.args[1];
            renameDirs.insert(std::make_pair(path, to));
            std::set<std::string> newFiles_tmp = newFiles;
            std::set<std::string> newDirs_tmp = newDirs;

            for (auto p:newFiles_tmp) {
                if (isPrefix(p, path)) {
                    newFiles.erase(p);
                    p.replace(0, path.size(), to);
                    newFiles.insert(p);
                }
            }
            for (auto p:newDirs_tmp) {
                if (isPrefix(p, path)) {
                    newDirs.erase(p);
                    p.replace(0, path.size(), to);
                    newDirs.insert(p);
                }
            }
        }
    }


    if (!newFiles.empty() || !deleteFiles.empty()
        || !newDirs.empty() || !deleteDirs.empty() || !renameDirs.empty()) {

        auto newStamp = host->networkAgent->pushToServer(newFiles, deleteFiles,
                                         newDirs, deleteDirs, renameDirs);
        dynamic_cast<DriveClientAgent*>(host)-> last_sync = newStamp;
        printf("PUSH done, last_sync=%lu\n", newStamp);
    }
    /// moved
//    auto stamp = host->networkAgent->pullfromServer(host->last_sync);
//    host->last_sync = stamp;
    printf("[DriveClientAgent::BackgroundUpdater::update] done\n");

}
