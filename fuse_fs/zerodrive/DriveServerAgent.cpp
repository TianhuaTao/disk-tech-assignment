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
#include "zerodrive_common.h"
#include "DriveServerAgent.h"
#include <fstream>

DriveServerAgent::DriveServerAgent(const char *address, int port) {
    networkAgent = new NetworkAgent(this);
    networkAgent->setRole(NetworkAgent::SERVER);
    networkAgent->listenAsync(address, port, []() {
        std::cout << "Got connection\n";
    });

    fileOperation = new FileOperation();
    backgroundTask = new BackgroundTask(this);
    backgroundTask->run();

    server_stamp = getLatestStamp();
    if (server_stamp == 0)// no previous record
        server_stamp = ZeroDrive::getTimestamp();
    printf("DriveServerAgent init complete\n");
}

DriveServerAgent::~DriveServerAgent() {
    delete networkAgent;
    delete fileOperation;
    printf("Server agent shut down\n");
}

void *DriveServerAgent::Init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
    return fileOperation->Init(conn, cfg);
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
    return fileOperation->Getattr(path, stbuf, fi);
    //TODO: send signal to client
    // send signal to client
    //std::vector<std::string> detail;
    //detail.push_back(std::string(path));
    //this->broadcastChanges(WRITE_DONE, detail);
}

//---------------------will do something-----------------------------------------------------------------

int DriveServerAgent::Write(const char *path, const char *buf, size_t size,
                            off_t offset, struct fuse_file_info *fi) {
    int ret = fileOperation->Write(path, buf, size, offset, fi);
    // send signal to client
    std::vector<std::string> detail;
    detail.emplace_back(path);
    if (ret >= 0) {
        OperationRecord r = OperationRecord(WRITE_DONE, detail);
        backgroundTask->addJournal(r);
    }
    return ret;
}

int DriveServerAgent::Rename(const char *from, const char *to, unsigned int flags) {
    // send signal to client

    std::vector<std::string> detail;
    detail.emplace_back(from);
    detail.emplace_back(to);
    bool isDir = fileOperation->checkIsDir(from);
    int ret = fileOperation->Rename(from, to, flags);
    if (ret >= 0) {
        OperationRecord r = OperationRecord(isDir ? RENAME_DIR : RENAME, detail);
        backgroundTask->addJournal(r);
    }
    return ret;
}

int DriveServerAgent::Open(const char *path, struct fuse_file_info *fi) {
    bool exist_before = fileOperation->checkExist(path);
    int ret = fileOperation->Open(path, fi);
    if (!exist_before && ret >= 0) { // newly created
        OperationRecord r = OperationRecord(OPEN, path);
        backgroundTask->addJournal(r);
    }
    return ret;
}

int
DriveServerAgent::Readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi,
                          enum fuse_readdir_flags flags) {
    return fileOperation->Readdir(path, buf, filler, offset, fi, flags);
}

int DriveServerAgent::Create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    std::vector<std::string> detail;
    detail.emplace_back(path);
    detail.push_back(std::to_string(mode));
    int ret = fileOperation->Create(path, mode, fi);
    if (ret >= 0) {
        OperationRecord r = OperationRecord(CREATE, detail);
        backgroundTask->addJournal(r);
    }
    return ret;
}

int DriveServerAgent::Mkdir(const char *path, mode_t mode) {
    int ret = fileOperation->Mkdir(path, mode);
    std::vector<std::string> detail;
    detail.emplace_back(path);
    detail.push_back(std::to_string(mode));
    if (ret >= 0) {
        OperationRecord r = OperationRecord(MKDIR, detail);
        backgroundTask->addJournal(r);
    }
    return ret;
}

int DriveServerAgent::Rmdir(const char *path) {
    int ret = fileOperation->Rmdir(path);
    std::vector<std::string> detail;
    detail.emplace_back(path);
    if (ret >= 0) {
        OperationRecord r = OperationRecord(RMDIR, detail);
        backgroundTask->addJournal(r);
    }
    return ret;
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
    int ret = fileOperation->Chmod(path, mode, fi);
    // send signal to client
    std::vector<std::string> detail;
    detail.emplace_back(path);
    detail.push_back(std::to_string(mode));
//    this->broadcastChanges(CHMOD, detail);

    return ret;
}

int DriveServerAgent::Chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi) {
    // TODO: send signal to client
    // send signal to client
    std::vector<std::string> detail;
    detail.emplace_back(path);
    detail.push_back(std::to_string(uid));
    detail.push_back(std::to_string(gid));
//    this->broadcastChanges(CHOWN, detail);

    return fileOperation->Chown(path, uid, gid, fi);
}

int DriveServerAgent::Readlink(const char *path, char *buf, size_t size) {
    return fileOperation->Readlink(path, buf, size);

}

//int DriveServerAgent::broadcastChanges(enum Operation_t msg, std::vector<std::string> detail) {
//    if(msg>0&&msg<10){
//        assert(detail.size() == (unsigned  long)Message_Number[msg]);
//        networkAgent->sendMessageToAll(msg, detail);
//    }else
//    {
//        std::cout << "Operation_t not supported\n";
//    }
//    return 0;
//}

uint64_t DriveServerAgent::getServerStamp() const {
    return server_stamp;
}

int DriveServerAgent::Unlink(const char *path) {
    int ret = fileOperation->Unlink(path);
    std::vector<std::string> detail;
    detail.emplace_back(path);
    if (ret >= 0) {
        OperationRecord r = OperationRecord(UNLINK, detail);
        backgroundTask->addJournal(r);
    }
    return ret;
}

void DriveServerAgent::handleUpdate(int connection_fd, const std::vector<std::string> &newFiles,
                                    const std::vector<std::string> &deleteFiles,
                                    const std::vector<std::string> &newDirs,
                                    const std::vector<std::string> &deleteDirs,
                                    const std::vector<std::pair<std::string, std::string>> &renameDirs) {
    printf("[DriveServerAgent] Processing updates\n");
    // TODO: this is buggy
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
        OperationRecord r = OperationRecord(WRITE_DONE, path);
        backgroundTask->addJournal(r);
    }
    for (const auto &p: renameDirs) {
//       fileOperation->Rename(p.first.c_str(),p.second.c_str(), 0);
        this->Rename(p.first.c_str(), p.second.c_str(), 0);
    }

//    set_server_stamp_as_now();
    backgroundTask->stageChanges();
    networkAgent->sendRaw(connection_fd, (char *) &server_stamp, sizeof(server_stamp));
}

void DriveServerAgent::handlePull(int sockfd, uint64_t last_sync) {
    if (last_sync == server_stamp) {
        Operation_t token = UP_TO_DATE;
        networkAgent->sendRaw(sockfd, (char *) &token, sizeof token);
        return;
    }
    if (last_sync == 0) {
        // completely new client
        // download all files

        networkAgent->sendToken(sockfd, DOWNLOAD_ALL);
        std::cout << "[debug] respond DOWNLOAD_ALL" << "\n";

        networkAgent->sendAllData(sockfd);
    } else {
        /// corresponding to NetworkAgent::pullfromServer -- PATCH
        Operation_t token = PATCH;
        std::cout << "[debug] respond PATCH" << "\n";
        networkAgent->sendRaw(sockfd, (char *) &token, sizeof token);
        // find diff journals
        std::string possilbe_journal_file = std::to_string(last_sync);
        auto all_journal_files = fileOperation->getJournalList();
        std::vector<std::string> diff;
        for (auto &f: all_journal_files) {
            if (f > possilbe_journal_file) {
                diff.push_back(f);
                std::cout << "[debug] Journal file " << f << "\n";
            }
        }
        std::sort(diff.begin(), diff.end());
        printf("[DriveServerAgent::handlePull] %zu commits\n", diff.size());

        // merge commits and transfer them
        std::queue<OperationRecord> combined;
        for (const auto &f: all_journal_files) {
            auto records = readJournal(f);
            for (auto &r :records) {
                combined.push(r);
            }
        }

        // process and merge
        using namespace ZeroDrive;
        std::set<std::string> newFiles;     // the files that are modified
        std::set<std::string> deleteFiles;
        std::set<std::string> newDirs;
        std::set<std::string> deleteDirs;
        std::set<std::pair<std::string, std::string>> renameDirs;
        while (!combined.empty()) {
            auto c = combined.front();
            combined.pop();
            auto path = c.args[0];
            auto op = c.type;
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
            printf("Merge complete, ready to send\n");

        }
        networkAgent->pushToClient(sockfd, newFiles, deleteFiles,
                                   newDirs, deleteDirs, renameDirs);
        printf("PUSH done\n");

        networkAgent->sendRaw(sockfd, (char *) &server_stamp, sizeof(server_stamp));


    }
}

uint64_t DriveServerAgent::getLatestStamp() {
    auto ent = fileOperation->getJournalList();
    if (ent.empty())return 0;
    std::sort(ent.begin(), ent.end());
    std::cout << ent[ent.size() - 1] << "\n";
    return std::stoull(ent[ent.size() - 1]);
}

std::vector<OperationRecord> DriveServerAgent::readJournal(const std::string &path) {

#define GET_N_ARGS(n, ifs, container)   \
        printf("[readJournal] token=%d ", token);\
    for (int i = 0; i < (n); ++i){ \
        std::string tmp;\
        std::getline(ifs, tmp);\
        container.push_back(tmp);\
        printf("arg=%s ", tmp.c_str());\
    }\
     printf("\n");

    std::vector<OperationRecord> ret;
    auto absPath = std::string(ZeroDrive::get_journal_dir()) + "/" + path;
    std::ifstream ifs(absPath);
    int32_t token_i;
    Operation_t token;
    std::string t;  // to get rid of the '\n'
    while (ifs >> token_i) {

        std::getline(ifs, t); // a remaining newline

        token = (Operation_t) token_i;
        std::vector<std::string> args;

        if (token == WRITE_DONE) {
            GET_N_ARGS(1, ifs, args)
        } else if (token == RENAME) {
            GET_N_ARGS(2, ifs, args)
        } else if (token == MKDIR) {
            GET_N_ARGS(2, ifs, args)
        } else if (token == RMDIR) {
            GET_N_ARGS(1, ifs, args)
        } else if (token == CREATE) {
            GET_N_ARGS(2, ifs, args)
        } else if (token == OPEN) {
            GET_N_ARGS(1, ifs, args)
        } else if (token == UNLINK) {
            GET_N_ARGS(1, ifs, args)
        } else if (token == RENAME_DIR) {
            GET_N_ARGS(2, ifs, args)
        } else {
            printf("[DriveServerAgent::readJournal] unknown token %d\n", token_i);
        }
        ret.emplace_back(token, args);
    }
    return ret;

#undef GET_N_ARGS
}

uint64_t DriveServerAgent::set_server_stamp_as_now() {
    return set_server_stamp_as(ZeroDrive::getTimestamp());
}

uint64_t DriveServerAgent::set_server_stamp_as(uint64_t newStamp) {
    std::unique_lock<std::mutex> mlock(stamp_mutex);
    server_stamp = newStamp;
    return server_stamp;
}

void DriveServerAgent::BackgroundTask::run() {
    running = true;
    // start a thread to stage changes every 10 seconds
    stageChangeThread = new std::thread([&]() {
        while (this->running) {
            stageChanges();
            ZeroDrive::sleep_seconds(4);
        }
    });
}

DriveServerAgent::BackgroundTask::BackgroundTask(DriveServerAgent *driveServerAgent) : host(driveServerAgent) {

}

void DriveServerAgent::BackgroundTask::stageChanges() {
    printf("Staging %d changes\n", unstagedChanges.size());
    if (unstagedChanges.size() > 0) {
        auto new_stamp = ZeroDrive::getTimestamp();
        auto j_filename = std::string(ZeroDrive::get_journal_dir()) + "/" + std::to_string(new_stamp);
        std::ofstream fs(j_filename);

        while (unstagedChanges.size() > 0) {
            auto r = unstagedChanges.front();
            unstagedChanges.pop_front();
            fs << r.type << "\n";
            for (const auto &s: r.args) {
                fs << s << "\n";
            }
        }
        fs.close();
        host->server_stamp = new_stamp;   // update sync state
        printf("Stage changes, server_stamp=%ld\n", host->server_stamp);
    } else {
        printf("No changes, server_stamp=%ld\n", host->server_stamp);
    }
}

void DriveServerAgent::BackgroundTask::addJournal(OperationRecord &r) {
    unstagedChanges.push_back(r);
}

