sync_fs: sync_fs.c sync_fs_op.h sync_fs_op.c
	gcc -Wall sync_fs.c sync_fs_op.c `pkg-config fuse3 --cflags --libs` -o sync_fs

.PHONY:
clean:
	rm -rf *.o sync_fs
