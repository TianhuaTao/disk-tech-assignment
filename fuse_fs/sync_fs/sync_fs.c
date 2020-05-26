#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include "sync_fs_op.h"

static const struct fuse_operations sync_oper = {
    .init = sync_init,
    .getattr = sync_getattr,
    .readdir = sync_readdir,
    .open = sync_open,
    .read = sync_read,
    .rename = sync_rename,

};


static void server_init(){

}

static void client_init(){

}

static void show_help(const char *progname)
{

}


static struct options {
	int show_help;
    int local_port;
    int server;
    int client;
    int port;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--port=%d", port),
	OPTION("--server", server),
    OPTION("--client", client),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};


int main(int argc, char *argv[])
{
    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    /* Parse options */
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

    if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0][0] = '\0';
	}
        
    ret = fuse_main(args.argc, args.argv, &sync_oper, NULL);
    fuse_opt_free_args(&args);
    return ret;
}
