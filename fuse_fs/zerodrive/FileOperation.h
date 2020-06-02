#pragma once

#include <fuse.h>
#include <vector>
#include <string>
#include "Protocol.h"

class FileOperation {
private:

public:
    FileOperation();

    virtual ~FileOperation();

    virtual void *Init(struct fuse_conn_info *conn, struct fuse_config *cfg);

//-------------------------------------------------------------------------------
    virtual int Open(const char *path, struct fuse_file_info *fi);

    virtual int Read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi);

    virtual int Readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi,
                        enum fuse_readdir_flags flags);

    virtual int Readlink(const char *path, char *buf, size_t size);

    virtual int Getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi);

//----------------------------------------------------------------------------

    virtual int Create(const char *path, mode_t mode, struct fuse_file_info *fi);

    virtual int Write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

    virtual int Rename(const char *from, const char *to, unsigned int flags);

    virtual int Mkdir(const char *path, mode_t mode);

    virtual int Rmdir(const char *path);

    virtual int Chmod(const char *path, mode_t mode, struct fuse_file_info *fi);

    virtual int Chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi);

    virtual int Symlink(const char *from, const char *to);

    virtual int Unlink(const char *path);

    bool checkExist(const char *path);

    bool checkExistReal(const char *realpath);

    std::vector<std::string> getDirEntries(const char *realPath);

    bool checkIsDir(const char *path);

    std::vector<std::string> getJournalList();
};
