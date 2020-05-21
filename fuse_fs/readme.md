Ubuntu 安装 fuse:
sudo apt-get install fuse3 libfuse3-dev

编译单个c文件：
gcc -o hello hello.c `pkg-config fuse3 --cflags --libs`

编译c和cpp文件：
使用 makefile

运行helloworld：
./helloworld tmp

卸载：
fusermount3 -u tmp/

