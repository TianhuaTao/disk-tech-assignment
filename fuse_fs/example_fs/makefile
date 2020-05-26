example :example_fs.c
	gcc example_fs.c  `pkg-config fuse3 --cflags --libs` -o example

.PHONY:
clean:
	rm -rf example *.o *.a
