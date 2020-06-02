zerodrive: SharedQueue.h zerodrive.h zerodrive.cpp op.h op.cpp FileOperation.h FileOperation.cpp DriveAgent.h DriveAgent.cpp DriveClientAgent.h DriveClientAgent.cpp DriveServerAgent.h DriveServerAgent.cpp NetworkAgent.h NetworkAgent.cpp Protocol.h Protocol.cpp zerodrive_common.h zerodrive_common.cpp
	g++ -Wall zerodrive.cpp op.cpp FileOperation.cpp DriveAgent.cpp DriveClientAgent.cpp zerodrive_common.cpp DriveServerAgent.cpp NetworkAgent.cpp Protocol.cpp `pkg-config fuse3 --cflags --libs` -std=c++17 -o zerodrive

.PHONY:
clean:
	rm -rf *.o zerodrive