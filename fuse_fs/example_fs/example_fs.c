#define FUSE_USE_VERSION 31
#include <stdio.h>
#include <fuse.h>
#include <string.h>

/* 这里实现了一个遍历目录的功能，当用户在目录执行ls时，会回调到该函数，我们这里只是返回一个固定的文件Hello-world。 */
static int test_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi, enum fuse_readdir_flags flags)
{
    printf( "tfs_readdir	 path : %s ", path);
 
    return filler(buf, "Hello-world", NULL, 0, 0);
}

/* 显示文件属性 */
static int test_getattr(const char* path, struct stat *stbuf,struct fuse_file_info *fi)
{
    printf("tfs_getattr	 path : %s ", path);
    if(strcmp(path, "/") == 0)
        stbuf->st_mode = 0755 | S_IFDIR;
    else
        stbuf->st_mode = 0644 | S_IFREG;
    return 0;
}

/*这里是回调函数集合，这里实现的很简单*/
static struct fuse_operations tfs_ops = {
   .readdir = test_readdir,
   .getattr = test_getattr,
};

int main(int argc, char *argv[])
{
     int ret = 0;
     ret = fuse_main(argc, argv, &tfs_ops, NULL);
     return ret;
}