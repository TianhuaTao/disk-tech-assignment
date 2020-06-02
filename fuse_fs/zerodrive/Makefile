cxx_files =     zerodrive.cpp\
                op.cpp\
                FileOperation.cpp\
                DriveAgent.cpp\
                DriveClientAgent.cpp\
                DriveServerAgent.cpp\
                NetworkAgent.cpp\
                Protocol.cpp\
                zerodrive_common.cpp

h_files =       SharedQueue.h\
                zerodrive.h\
                FileOperation.h\
                DriveAgent.h op.h\
                Protocol.h\
                DriveServerAgent.h\
                DriveClientAgent.h\
                zerodrive_common.h\
                NetworkAgent.h

zerodrive: $(h_files) $(cxx_files)
	g++ -Wall $(cxx_files) `pkg-config fuse3 --cflags --libs` -std=c++17 -o zerodrive

.PHONY:
clean:
	rm -rf *.o zerodrive