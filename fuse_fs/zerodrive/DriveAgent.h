#pragma once
#include <fuse.h>

class NetworkAgent;

class DriveAgent {
private:

protected:
    NetworkAgent *networkAgent = nullptr;

public:
    DriveAgent() = default;
    virtual ~DriveAgent() = default;

    virtual int Rename(const char *from, const char *to, unsigned int flags) = 0;
    virtual int Open(const char *path, struct fuse_file_info *fi) = 0;
    virtual int Read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi) = 0;
    virtual int Write(const char *path, const char *buf, size_t size,
                      off_t offset, struct fuse_file_info *fi) = 0;
    virtual int Readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi,
                        enum fuse_readdir_flags flags) = 0;
    virtual void *Init(struct fuse_conn_info *conn,
                       struct fuse_config *cfg) = 0;
    virtual int Create(const char *path, mode_t mode,
                       struct fuse_file_info *fi) = 0;
    virtual int Getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) = 0;
    virtual int Mkdir(const char *path, mode_t mode) = 0;
    virtual int Rmdir(const char *path) = 0;
    virtual int Symlink(const char *from, const char *to) = 0;
    virtual int Chmod(const char *path, mode_t mode,
                      struct fuse_file_info *fi) = 0;
    virtual int Chown(const char *path, uid_t uid, gid_t gid,
                      struct fuse_file_info *fi) = 0;
    virtual int Readlink(const char *path, char *buf, size_t size) = 0;
};

class DriveServerAgent : public DriveAgent {
private:
public:
    DriveServerAgent(const char *address, int port);
    ~DriveServerAgent() override;

    int Write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) override;

    int Rename(const char *from, const char *to, unsigned int flags) override;

    int Open(const char *path, struct fuse_file_info *fi) override;

    int Mkdir(const char *path, mode_t mode) override;

    int Rmdir(const char *path) override;

    int Symlink(const char *from, const char *to) override;

    int Chmod(const char *path, mode_t mode, struct fuse_file_info *fi) override;

    int Chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi) override;

    int Readlink(const char *path, char *buf, size_t size) override;

    int Readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi,
                enum fuse_readdir_flags flags) override;

    int Create(const char *path, mode_t mode, struct fuse_file_info *fi) override;

    void *Init(struct fuse_conn_info *conn,
               struct fuse_config *cfg) override;

    int Getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) override;

    int Read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) override;

private:
    int broadcastChanges();
};

class DriveClientAgent : public DriveAgent {
private:
public:
    DriveClientAgent(const char *address, int port);
    ~DriveClientAgent() override;

    int Rename(const char *from, const char *to, unsigned int flags) override;

    int Open(const char *path, struct fuse_file_info *fi) override;

    int Read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) override;

    int Write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) override;

    int Mkdir(const char *path, mode_t mode) override;

    int Rmdir(const char *path) override;

    int Symlink(const char *from, const char *to) override;

    int Chmod(const char *path, mode_t mode, struct fuse_file_info *fi) override;

    int Chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi) override;

    int Readlink(const char *path, char *buf, size_t size) override;

    int Readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi,
                enum fuse_readdir_flags flags) override;

    int Create(const char *path, mode_t mode, struct fuse_file_info *fi) override;

    int Getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) override;

    void *Init(struct fuse_conn_info *conn,
               struct fuse_config *cfg) override;
};
