#ifndef __FUTILS_H__
#define __FUTILS_H__

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAX_FILE_NAME (1 << 8)
#define MAX_DIR_NAME 5000
#define OPEN_DIR_MODE (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)

DIR *openDir(const char[]);
void closeDir(DIR*);
int getFD(const char *const, int, int);
void closeFD(int);
bool hasRights(mode_t);
void writeHelper(int, void*, size_t);
ssize_t readHelper(int, void*, size_t);

#endif