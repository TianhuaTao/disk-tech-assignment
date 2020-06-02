#define FUSE_USE_VERSION 31


#include "DriveAgent.h"
#include "NetworkAgent.h"
#include <iostream>
#include "op.h"
#include "Protocol.h"

void DriveAgent::freeSocket() {
    networkAgent->freeSocket();
}

int DriveAgent::Unlink(const char *path) {
    return 0;
}

void DriveAgent::lock() {

}
