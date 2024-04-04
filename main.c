#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>

#include "argparser.h"

#define MAX_DIR_NAME 5000
#define OPEN_DIR_MODE (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)

DIR *openDir(const char name[]) {
  DIR *dir = opendir(name);
  if (dir == NULL) {
    perror("opendir");
    exit(1);
  }
  return dir;
}

void closeDir(DIR *dir) {
  if (closedir(dir) == -1) {
    perror("closedir");
  }
}

int getFD(const char *const fname, int flags, int perm) {
  int fd = open(fname, flags, perm);
  if (fd < 0) {
    perror("");
    exit(1);
  }
  return fd;
}
 
void closeFD(int fd) {
  if (close(fd) < 0) {
    perror("");
    exit(1);
  }
}

char *getCurrDateTime() {
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  static char timeStr[100]; 
  sprintf(timeStr, "%d-%02d-%02d_%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  return timeStr;
}

void iterDirRec(const char dirname[]) {
  DIR *dir = openDir(dirname);
  for (struct dirent *deBuff; (deBuff = readdir(dir)) != NULL;) {
    char *d_name = deBuff->d_name;
    if (!strcmp(d_name, ".vcs") || !strcmp(d_name, ".") || !strcmp(d_name, "..")) {
      continue;
    }
    puts(d_name);
    struct stat statBuff;
    stat(d_name, &statBuff);
    if (S_ISDIR(statBuff.st_mode)) {
      iterDirRec(d_name);
    } else {
      
    }
  }
  closeDir(dir);
}

void snapshot() {
  char snapTarget[100] = "./.vcs/snapshot_";
  strcat(snapTarget, getCurrDateTime());
  if (mkdir(snapTarget, OPEN_DIR_MODE) == -1) {
      perror("mkdir-snap");
      exit(1);
  }
  //copy
  static char command[200];
  sprintf(command, "rsync -av --progress . %s --exclude .vcs", snapTarget);
  system(command);
  //save
  iterDirRec(".");
}

void solve(const char dirname[]) {
  if (chdir(dirname)) {
    perror("chdir");
    exit(1);
  }
  
  DIR *vcsDir;
  if ((vcsDir = opendir(".vcs")) == NULL && errno == ENOENT) {
    if (mkdir(".vcs", OPEN_DIR_MODE) == -1) {
      perror("mkdir");
      exit(1);
    }
  }

  snapshot();
  if (vcsDir != NULL) {
    closeDir(vcsDir);
  }
}

void wrongUsage() {
  fprintf(stderr, "Wrong usage!\n");
  exit(1);
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    wrongUsage();
  }

  printf("%d\n", argc);
  Args args;
  initArgs(argc, argv, &args);
  

  ArgPair *targets = getVal(&args, "-t");
  if (targets == NULL) {
    wrongUsage();
  }

  for (int i = 0; i < targets->cnt; i++) {
    solve(targets->values[i]);
  }
  return 0;
}
