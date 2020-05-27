#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include "sync_fs_op.h"
#include "sync_fs_cxx_api.h"
static const struct fuse_operations sync_oper = {
    .init = sync_init,
    .getattr = sync_getattr,
    .readdir = sync_readdir,
    .open = sync_open,
    .read = sync_read,
    .rename = sync_rename,
    .link = sync_symlink,
    .mkdir = sync_mkdir,
    .readlink = sync_readlink,

};

static struct options
{
    int show_help;
    // int local_port;
    int server;
    int client;
    int port;
    const char* address;
} options;

 void server_init()
{
    server_init_cxx(address, port);
    printf("Server init complete\n");
}

 void client_init()
{

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
    OPTION("--address=<s>", address),
    OPTION("-h", show_help),
    OPTION("--help", show_help),
    FUSE_OPT_END};

void inputPassword(char buf[])
{
    printf("Enter password:\n");
    scanf("%s", buf);
}

int check_passwd(){
    // TODO: add check password
    return 0;
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

    if (options.server & options.client)
    {
        printf("Cannot be server and client at the same time\n");
        fuse_opt_free_args(&args);
        return -1;
    }

    char passwd_buf[512];
    inputPassword(passwd_buf);
    if (check_passwd(passwd_buf)==0)
    {
        // check good
    }
    else
    {
        printf("Wrong password\n");
        fuse_opt_free_args(&args);
        return 1;
    }

    if (options.server)
    {
        server_init();
    }
    else
    {
        client_init();
    }

    ret = fuse_main(args.argc, args.argv, &sync_oper, NULL);
    fuse_opt_free_args(&args);
    return ret;
}
