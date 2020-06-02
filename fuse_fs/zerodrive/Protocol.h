#pragma once
#include <string>
#include <utility>
#include <vector>

enum Operation_t{
    NONE,
    WRITE_DONE,//1
    RENAME,//2
    MKDIR,//2
    RMDIR,//1
    CREATE,//2
    CHMOD,//2
    CHOWN,//3
    REMOVE,
    CREATE_DIR,
    REQUEST_FILE, // 1
    OPEN,
    UNLINK,
    UPDATE,
    DELETE,
    PUSH,
    RENAME_DIR,
    PULL,
    UP_TO_DATE,
    DOWNLOAD_ALL,
    PATCH,
};

struct OperationRecord{
    Operation_t type;
    std::vector<std::string> args;

    OperationRecord(Operation_t t, const std::string& arg):type(t){
        args.push_back(arg);
    }
    OperationRecord(Operation_t t, std::vector<std::string> a):type(t),args(a){

    }
};

//extern int Message_Number[11];