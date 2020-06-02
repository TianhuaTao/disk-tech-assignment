#pragma once
#include <string>
#include <utility>
#include <vector>

enum Operation_t{
    NONE,
    WRITE_DONE,
    RENAME,
    MKDIR,
    RMDIR,
    CREATE,
    CHMOD,
    CHOWN,
    REMOVE,
    CREATE_DIR,
    REQUEST_FILE,
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
    OperationRecord(Operation_t t, const std::vector<std::string>& a):type(t),args(a){

    }
};

//extern int Message_Number[11];