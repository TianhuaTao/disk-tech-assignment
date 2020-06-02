#include "DriveAgent.h"
#include "SharedQueue.h"

class DriveClientAgent : public DriveAgent {

private:
    struct BackgroundUpdater{
        DriveClientAgent* host;
        explicit BackgroundUpdater(DriveClientAgent* driveClientAgent);
        void run();
        std::thread *updatingThread = nullptr;
        bool running = false;

        void update();
        SharedQueue<OperationRecord> dirtyChanges;


    };

    BackgroundUpdater *backgroundUpdater;
    uint64_t client_stamp{};
    uint64_t last_sync{};
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

    int Unlink(const char *path) override;

//    void onMsgWriteDone(std::string path) override;
//
//    void onMsgCreate(std::string path, mode_t mode) override;
//
//    void onMsgMkdir(std::string path, mode_t mode) override;
//
//    void onMsgRename(std::string from, std::string to) override;
//
//    void onMsgRmdir(std::string path) override;
//
//    void onMsgChmod(std::string path, mode_t mode) override;

private:
//    int broadcastChanges(enum Operation_t msg, std::vector<std::string> detail);
};
