#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <fcntl.h>
#include <cstddef>
#include <cassert>
#include "zerodrive.h"
#include "DriveAgent.h"


DriveAgent* localAgent;
static const struct fuse_operations sync_oper = {

    sync_getattr, // int (*getattr) (const char *, struct stat *, struct fuse_file_info *fi);

    sync_readlink, // int (*readlink) (const char *, char *, size_t);

    nullptr, // int (*mknod) (const char *, mode_t, dev_t);

    sync_mkdir, // int (*mkdir) (const char *, mode_t);

    nullptr, // int (*unlink) (const char *);

    sync_rmdir, // int (*rmdir) (const char *);

    sync_symlink, // int (*symlink) (const char *, const char *);

    sync_rename, // int (*rename) (const char *, const char *, unsigned int flags);

    sync_link, // int (*link) (const char *, const char *);

    sync_chmod, // int (*chmod) (const char *, mode_t, struct fuse_file_info *fi);

    sync_chown, // int (*chown) (const char *, uid_t, gid_t, struct fuse_file_info *fi);

    sync_truncate, // int (*truncate) (const char *, off_t, struct fuse_file_info *fi);

    sync_open, // int (*open) (const char *, struct fuse_file_info *);

    sync_read, // int (*read) (const char *, char *, size_t, off_t,
    // 	     struct fuse_file_info *);

    sync_write, // int (*write) (const char *, const char *, size_t, off_t, struct fuse_file_info *);

    nullptr, // int (*statfs) (const char *, struct statvfs *);

    nullptr, // int (*flush) (const char *, struct fuse_file_info *);

    nullptr, // int (*release) (const char *, struct fuse_file_info *);

    nullptr, // int (*fsync) (const char *, int, struct fuse_file_info *);

    nullptr, // int (*setxattr) (const char *, const char *, const char *, size_t, int);

    nullptr, // int (*getxattr) (const char *, const char *, char *, size_t);

    nullptr, // int (*listxattr) (const char *, char *, size_t);

    nullptr, // int (*removexattr) (const char *, const char *);

    nullptr, // int (*opendir) (const char *, struct fuse_file_info *);

    sync_readdir, // int (*readdir) (const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *, enum fuse_readdir_flags);

    nullptr, // int (*releasedir) (const char *, struct fuse_file_info *);

    nullptr, // int (*fsyncdir) (const char *, int, struct fuse_file_info *);

    sync_init, // void *(*init) (struct fuse_conn_info *conn, struct fuse_config *cfg);

    nullptr, // void (*destroy) (void *private_data);

    nullptr, // int (*access) (const char *, int);

    sync_create, // int (*create) (const char *, mode_t, struct fuse_file_info *);

    nullptr, // int (*lock) (const char *, struct fuse_file_info *, int cmd, struct flock *);

    nullptr, //  int (*utimens) (const char *, const struct timespec tv[2], struct fuse_file_info *fi);

    nullptr, // int (*bmap) (const char *, size_t blocksize, uint64_t *idx);

    nullptr, // 	int (*ioctl) (const char *, int cmd, void *arg, struct fuse_file_info *, unsigned int flags, void *data);

    nullptr, // int (*poll) (const char *, struct fuse_file_info *, struct fuse_pollhandle *ph, unsigned *reventsp);

    nullptr, // int (*write_buf) (const char *, struct fuse_bufvec *buf, off_t off, struct fuse_file_info *);

    nullptr, // int (*read_buf) (const char *, struct fuse_bufvec **bufp, size_t size, off_t off, struct fuse_file_info *);

    nullptr, // int (*flock) (const char *, struct fuse_file_info *, int op);

    nullptr, // int (*fallocate) (const char *, int, off_t, off_t, struct fuse_file_info *);

    nullptr, // ssize_t (*copy_file_range) (const char *path_in,
    // 			    struct fuse_file_info *fi_in,
    // 			    off_t offset_in, const char *path_out,
    // 			    struct fuse_file_info *fi_out,
    // 			    off_t offset_out, size_t size, int flags);

    nullptr, // off_t (*lseek) (const char *, off_t off, int whence, struct fuse_file_info *);
};

static struct options
{
    int show_help;
    // int local_port;
    int server;
    int client;
    int port;
    const char *address;
    const char *prefix;
} options;

const char* global_prefix;

void server_init()
{
    localAgent = new DriveServerAgent(options.address, options.port);
    printf("Server init complete\n");
}

void client_init()
{
    localAgent = new DriveClientAgent(options.address, options.port);
    printf("Client init complete\n");
}

static void show_help(const char *progname)
{
    printf("usage: %s [options] <mountpoint>\n\n", progname);
    printf("File-system specific options:\n"
           "    --server          Start as server disk\n"
           "    --client          Start as client disk\n"
           "    --address=<s>     Server address\n"
           "    --port=<d>        Server port\n"
           "\n");
}

#define OPTION(t, p)                      \
    {                                     \
        t, offsetof(struct options, p), 1 \
    }
static const struct fuse_opt option_spec[] = {
    OPTION("--port=%d", port),
    OPTION("--server", server),
    OPTION("--client", client),
    OPTION("--address=%s", address),
    OPTION("--prefix=%s", prefix),
    OPTION("-h", show_help),
    OPTION("--help", show_help),
    FUSE_OPT_END};

std::string inputPassword()
{
    std::string result;
    printf("Enter password:\n");
    std::cin >> result;
    return result;
}

bool check_passwd(const std::string &passwd)
{
    // TODO: add check password
    return true;
}

int main(int argc, char *argv[])
{
    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    /* Parse options */
    if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
        return 1;

    if (options.show_help)
    {
        show_help(argv[0]);
        assert(fuse_opt_add_arg(&args, "--help") == 0);
        args.argv[0][0] = '\0';
    }

    if(options.prefix){
        global_prefix= strdup((std::string("/")+ options.prefix).c_str());
    }else{
        global_prefix= strdup("/zerodrive_internal");
    }
    printf("The prefix is %s\n", global_prefix);
    if (options.server && options.client)
    {
        printf("Cannot be server and client at the same time\n");
        fuse_opt_free_args(&args);
        return -1;
    }

    if (options.server)
    {
        server_init();
    }
    else
    {
        auto passwd = inputPassword();
        if (check_passwd(passwd))
        {
            // check good
        }
        else
        {
            printf("Wrong password\n");
            fuse_opt_free_args(&args);
            return 1;
        }
        client_init();
    }
    ret = fuse_main(args.argc, args.argv, &sync_oper, NULL);
    fuse_opt_free_args(&args);
    printf("Return\n");

    return ret;
}
