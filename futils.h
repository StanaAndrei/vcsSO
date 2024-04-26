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

DIR *openDir(const char name[]);
void closeDir(DIR *dir);
int getFD(const char *const fname, int flags, int perm);
void closeFD(int fd);
bool hasRights(mode_t perm);

#endif