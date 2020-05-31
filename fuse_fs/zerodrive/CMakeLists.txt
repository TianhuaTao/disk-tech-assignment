cmake_minimum_required(VERSION 3.16)
project(zerodrive)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(/usr/local/include/fuse3)

add_executable(zerodrive
        DriveAgent.cpp
        DriveAgent.h
        DriveClientAgent.cpp
        DriveClientAgent.h
        DriveServerAgent.cpp
        DriveServerAgent.h
        op.cpp
        op.h
        zerodrive.cpp
        zerodrive.h
        Protocol.h NetworkAgent.cpp)
