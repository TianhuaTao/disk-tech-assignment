//
// Created by sam on 2020/6/1.
//

#ifndef ZERODRIVE_ZERODRIVE_COMMON_H
#define ZERODRIVE_ZERODRIVE_COMMON_H

#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cerrno>
#include <sys/types.h>
#include <pwd.h>
#include <algorithm>
#include <utility>
#include <regex>
#include "SharedQueue.h"

namespace ZeroDrive {
    inline uint64_t getTimestamp() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }

    inline void sleep_milliseconds(int x) {
        std::this_thread::sleep_for(std::chrono::milliseconds(x));
    }

    inline void sleep_seconds(int x) {
        sleep_milliseconds(1000 * x);
    }

    const char *get_homedir();

    const char *get_data_dir();

    const char *get_tmp_dir();

    const char *get_journal_dir();


#define CONVERT_PATH(newName, path) \
        char newName [512];\
        strcpy(newName, get_data_dir());\
        strcat(newName, path);

    inline bool isPrefix(std::string target, std::string prefix) {
        auto res = std::mismatch(prefix.begin(), prefix.end(), target.begin());
        return res.first == prefix.end();
    }
}


#endif //ZERODRIVE_ZERODRIVE_COMMON_H
