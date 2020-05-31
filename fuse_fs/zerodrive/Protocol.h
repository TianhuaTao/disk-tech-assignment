#pragma once

enum Message{
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
};

extern int Message_Number[11];