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

void *DriveServerAgent::Init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
    (void) conn;
    cfg->use_ino = 1;

    /* Pick up changes from lower filesystem right away. This is
	   also necessary for better hardlink support. When the kernel
	   calls the unlink() handler, it does not know the inode of
	   the to-be-removed entry and can therefore not invalidate
	   the cache of the associated inode - resulting in an
	   incorrect st_nlink value being reported for any remaining
	   hardlinks to this inode. */
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
}

int DriveServerAgent::Read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int fd;
    int res;
    printf("[debug] read\n");
    CONVERT_PATH(real_path, path);
    if (fi == nullptr)
        fd = open(real_path, O_RDONLY);
    else
        fd = fi->fh;

    if (fd == -1)
        return -errno;

    res = pread(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    if (fi == nullptr)
        close(fd);


    return res;
}

int DriveServerAgent::Getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void) fi;
    int res;
    CONVERT_PATH(real_path, path)
    res = lstat(real_path, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}

int DriveServerAgent::Write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int fd;
    int res;
    printf("[debug] write\n");
    CONVERT_PATH(real_path, path);

    (void) fi;
    if (fi == nullptr)
        fd = open(real_path, O_WRONLY);
    else
        fd = fi->fh;

    if (fd == -1)
        return -errno;

    res = pwrite(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    if (fi == nullptr)
        close(fd);

    // send signal to client
    std::vector<std::string> detail;
    detail.push_back(std::string(path));
    this->broadcastChanges(WRITE_DONE, detail);

    return res;
}

int DriveServerAgent::Rename(const char *from, const char *to, unsigned int flags) {
    int res;
    CONVERT_PATH(real_from, from)
    CONVERT_PATH(real_to, to)
    if (flags)
        return -EINVAL;

    res = rename(real_from, real_to);
    if (res == -1)
        return -errno;

    // TODO: send signal to client
    return 0;
}

int DriveServerAgent::Open(const char *path, struct fuse_file_info *fi) {
    int res;
    printf("[debug] open\n");
    CONVERT_PATH(real_path, path);
    res = open(real_path, fi->flags);
    if (res == -1)
        return -errno;

    fi->fh = res;
    return 0;
}

int
DriveServerAgent::Readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi,
                          enum fuse_readdir_flags flags) {
    DIR *dp;
    struct dirent *de;

    (void) offset;
    (void) fi;
    (void) flags;

    CONVERT_PATH(real_path, path);

    dp = opendir(real_path);
    if (dp == nullptr)
        return -errno;

    while ((de = readdir(dp)) != nullptr) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0, (fuse_fill_dir_flags) 0))
            break;
    }

    closedir(dp);
    return 0;
}

int DriveServerAgent::Create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    int res;
    CONVERT_PATH(real_path, path);

    res = open(real_path, fi->flags, mode);
    if (res == -1)
        return -errno;

    fi->fh = res;
    // TODO: send signal to client
    return 0;
}

DriveServerAgent::DriveServerAgent(const char *address, int port) {
    networkAgent = new NetworkAgent();
    networkAgent->setRole(NetworkAgent::SERVER);
    networkAgent->listenAsync(address, port, []() {
        std::cout << "Got connection\n";
    });
    printf("NetworkAgent init complete\n");

}

DriveServerAgent::~DriveServerAgent() {
    printf("Server agent shut down\n");
}

int DriveServerAgent::Mkdir(const char *path, mode_t mode) {
    int res;
    CONVERT_PATH(real_path, path)

    res = mkdir(real_path, mode);
    if (res == -1)
        return -errno;

    // TODO: send signal to client
    return 0;
}

int DriveServerAgent::Rmdir(const char *path) {
    int res;
    CONVERT_PATH(real_path, path)

    res = rmdir(real_path);
    if (res == -1)
        return -errno;

    // TODO: send signal to client
    return 0;
}

int DriveServerAgent::Symlink(const char *from, const char *to) {
    int res;
    CONVERT_PATH(real_from, from)
    CONVERT_PATH(real_to, to)

    res = symlink(real_from, real_to);
    if (res == -1)
        return -errno;

    // TODO: send signal to client
    return 0;
}

int DriveServerAgent::Chmod(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void) fi;
    int res;
    CONVERT_PATH(real_path, path);

    res = chmod(real_path, mode);
    if (res == -1)
        return -errno;

    // TODO: send signal to client
    return 0;
}

int DriveServerAgent::Chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi) {
    (void) fi;
    int res;
    CONVERT_PATH(real_path, path);
    res = lchown(real_path, uid, gid);
    if (res == -1)
        return -errno;

    // TODO: send signal to client
    return 0;
}

int DriveServerAgent::Readlink(const char *path, char *buf, size_t size) {
    int res;
    CONVERT_PATH(real_path, path)

    res = readlink(real_path, buf, size - 1);
    if (res == -1)
        return -errno;

    buf[res] = '\0';
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
            break;
        default:
            std::cout << "Message not supported\n";
            break;
    }


    return 0;
}

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
