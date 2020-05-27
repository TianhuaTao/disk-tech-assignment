zerodrive: zerodrive.h zerodrive.cpp op.h op.cpp DriveAgent.h DriveAgent.cpp
	g++ -Wall zerodrive.cpp op.cpp DriveAgent.cpp `pkg-config fuse3 --cflags --libs` -o zerodrive

.PHONY:
clean:
	rm -rf *.o zerodrive