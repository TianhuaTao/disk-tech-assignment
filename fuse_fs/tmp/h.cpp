/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPLv2.
  See the file COPYING.
*/

/** @file
 *
 * minimal example filesystem using high-level API
 *
 * Compile with:
 *
 *     g++ -Wall hello.c `pkg-config fuse3 --cflags --libs` -o hello
 *
 * ## Source code ##
 * \include hello.c
 */


#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */
static struct options {
	const char *filename;
	const char *contents;
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--name=%s", filename),
	OPTION("--contents=%s", contents),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static void *hello_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	(void) conn;
	cfg->kernel_cache = 1;
	return NULL;
}

static int hello_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	(void) fi;
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path+1, options.filename) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(options.contents);
	} else
		res = -ENOENT;

	return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags)
{
	(void) offset;
	(void) fi;
	(void) flags;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0, static_cast<fuse_fill_dir_flags>(0));
	filler(buf, "..", NULL, 0, (fuse_fill_dir_flags)0);
	filler(buf, options.filename, NULL, 0, (fuse_fill_dir_flags)0);

	return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path+1, options.filename) != 0)
		return -ENOENT;

	if ((fi->flags & O_ACCMODE) != O_RDONLY)
		return -EACCES;

	return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
	if(strcmp(path+1, options.filename) != 0)
		return -ENOENT;

	len = strlen(options.contents);
	if (offset < (long)len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, options.contents + offset, size);
	} else
		size = 0;

	return size;
}

static const struct fuse_operations hello_oper = {
	// int (*getattr) (const char *, struct stat *, struct fuse_file_info *fi);
	hello_getattr,

	// int (*readlink) (const char *, char *, size_t);
	nullptr,
	// int (*mknod) (const char *, mode_t, dev_t);
		nullptr,


	nullptr,// int (*mkdir) (const char *, mode_t);

	nullptr,// int (*unlink) (const char *);


	nullptr,// int (*rmdir) (const char *);


	nullptr,// int (*symlink) (const char *, const char *);


	nullptr,// int (*rename) (const char *, const char *, unsigned int flags);


	nullptr,// int (*link) (const char *, const char *);

	nullptr,// int (*chmod) (const char *, mode_t, struct fuse_file_info *fi);

	nullptr,// int (*chown) (const char *, uid_t, gid_t, struct fuse_file_info *fi);

	nullptr,// int (*truncate) (const char *, off_t, struct fuse_file_info *fi);


	hello_open,// int (*open) (const char *, struct fuse_file_info *);


	hello_read,// int (*read) (const char *, char *, size_t, off_t,
	// 	     struct fuse_file_info *);

	nullptr,// int (*write) (const char *, const char *, size_t, off_t,
	// 	      struct fuse_file_info *);


	nullptr,// int (*statfs) (const char *, struct statvfs *);

	nullptr,// int (*flush) (const char *, struct fuse_file_info *);

	
	nullptr,// int (*release) (const char *, struct fuse_file_info *);


	nullptr,// int (*fsync) (const char *, int, struct fuse_file_info *);

	nullptr,// int (*setxattr) (const char *, const char *, const char *, size_t, int);

	nullptr,// int (*getxattr) (const char *, const char *, char *, size_t);

	nullptr,// int (*listxattr) (const char *, char *, size_t);


	nullptr,// int (*removexattr) (const char *, const char *);


	nullptr,// int (*opendir) (const char *, struct fuse_file_info *);


	hello_readdir,// int (*readdir) (const char *, void *, fuse_fill_dir_t, off_t,
	// 		struct fuse_file_info *, enum fuse_readdir_flags);


	nullptr,// int (*releasedir) (const char *, struct fuse_file_info *);


	nullptr,// int (*fsyncdir) (const char *, int, struct fuse_file_info *);


	hello_init,// void *(*init) (struct fuse_conn_info *conn,
	// 	       struct fuse_config *cfg);

	nullptr,// void (*destroy) (void *private_data);


	nullptr,// int (*access) (const char *, int);

	nullptr,// int (*create) (const char *, mode_t, struct fuse_file_info *);


	nullptr,// int (*lock) (const char *, struct fuse_file_info *, int cmd,
	// 	     struct flock *);


	nullptr,//  int (*utimens) (const char *, const struct timespec tv[2],
	// 		 struct fuse_file_info *fi);


	nullptr,// int (*bmap) (const char *, size_t blocksize, uint64_t *idx);


	// #if FUSE_USE_VERSION < 35
	// 	int (*ioctl) (const char *, int cmd, void *arg,
	// 		      struct fuse_file_info *, unsigned int flags, void *data);
	// #else
	// 	int (*ioctl) (const char *, unsigned int cmd, void *arg,
	// 		      struct fuse_file_info *, unsigned int flags, void *data);
	// #endif
	nullptr,


	nullptr,// int (*poll) (const char *, struct fuse_file_info *,
	// 	     struct fuse_pollhandle *ph, unsigned *reventsp);


	nullptr,// int (*write_buf) (const char *, struct fuse_bufvec *buf, off_t off,
	// 		  struct fuse_file_info *);

	nullptr,// int (*read_buf) (const char *, struct fuse_bufvec **bufp,
	// 		 size_t size, off_t off, struct fuse_file_info *);

	nullptr,// int (*flock) (const char *, struct fuse_file_info *, int op);


	nullptr,// int (*fallocate) (const char *, int, off_t, off_t,
	// 		  struct fuse_file_info *);

	nullptr,// ssize_t (*copy_file_range) (const char *path_in,
	// 			    struct fuse_file_info *fi_in,
	// 			    off_t offset_in, const char *path_out,
	// 			    struct fuse_file_info *fi_out,
	// 			    off_t offset_out, size_t size, int flags);


	nullptr,// off_t (*lseek) (const char *, off_t off, int whence, struct fuse_file_info *);
};

static void show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n"
	       "    --name=<s>          Name of the \"hello\" file\n"
	       "                        (default: \"hello\")\n"
	       "    --contents=<s>      Contents \"hello\" file\n"
	       "                        (default \"Hello, World!\\n\")\n"
	       "\n");
}

int main(int argc, char *argv[])
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	/* Set defaults -- we have to use strdup so that
	   fuse_opt_parse can free the defaults if other
	   values are specified */
	options.filename = strdup("hello");
	options.contents = strdup("Hello World!\n");

	/* Parse options */
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	/* When --help is specified, first print our own file-system
	   specific help text, then signal fuse_main to show
	   additional help (by adding `--help` to the options again)
	   without usage: line (by setting argv[0] to the empty
	   string) */
	if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0][0] = '\0';
	}

	ret = fuse_main(args.argc, args.argv, &hello_oper, NULL);
	fuse_opt_free_args(&args);
	return ret;
}
