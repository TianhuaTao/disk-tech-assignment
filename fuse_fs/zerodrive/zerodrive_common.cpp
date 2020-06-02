//
// Created by sam on 2020/6/1.
//
#include "zerodrive_common.h"
extern const char *global_prefix;
namespace ZeroDrive {


    const char *get_homedir() {
        static int initialized = 0;
        static const char *homedir;
        if (!initialized) {
            struct passwd *pw = getpwuid(getuid());
            homedir = pw->pw_dir;
            initialized = 1;
        }
        return homedir;
    }

    const char *get_data_dir() {
        static int initialized = 0;
        static char datadir[512];
        if (!initialized) {
            memset(datadir, 0, sizeof datadir);
            strcat(datadir, get_homedir());
            strcat(datadir, global_prefix);
            initialized = 1;
        }
        return datadir;
    }

    const char *get_tmp_dir() {
        static int initialized = 0;
        static char tmpdir[512];
        if (!initialized) {
            memset(tmpdir, 0, sizeof tmpdir);
            strcat(tmpdir, "/tmp");
            strcat(tmpdir, global_prefix);
            mkdir(tmpdir, 0777);  // create dir
            // printf("Mkdir: %d\n" , ret);
            initialized = 1;
        }
        return tmpdir;
    }

    const char *get_journal_dir() {
        static int initialized = 0;
        static char journal_dir[512];
        if (!initialized) {
            memset(journal_dir, 0, sizeof journal_dir);
            strcat(journal_dir, get_homedir());
            strcat(journal_dir, global_prefix);
            strcat(journal_dir, ".journal");
            mkdir(journal_dir, 0777);  // create dir
            initialized = 1;
        }
        return journal_dir;
    }
}