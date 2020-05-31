zerodrive: zerodrive.h zerodrive.cpp op.h op.cpp FileOperation.h FileOperation.cpp DriveAgent.h DriveAgent.cpp DriveClientAgent.h DriveClientAgent.cpp DriveServerAgent.h DriveServerAgent.cpp NetworkAgent.h NetworkAgent.cpp Protocol.h
	g++ -Wall zerodrive.cpp op.cpp FileOperation.cpp DriveAgent.cpp DriveClientAgent.cpp DriveServerAgent.cpp NetworkAgent.cpp `pkg-config fuse3 --cflags --libs` -o zerodrive

.PHONY:
clean:
	rm -rf *.o zerodrive