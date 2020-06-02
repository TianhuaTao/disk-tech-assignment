#pragma once
#include <fuse.h>
#include <string>
#include <iostream>

const char *get_homedir();
const char *get_data_dir();
#define CONVERT_PATH(newName, path) \
		char newName [512];\
		strcpy(newName, get_data_dir());\
		strcat(newName, path);
