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

char *getCurrDateTime() {
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  static char timeStr[100]; 
  sprintf(timeStr, "%d-%02d-%02d_%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  return timeStr;
}

void snapshot() {
  char snapTarget[100] = "./.vcs/snapshot_";
  strcat(snapTarget, getCurrDateTime());
  if (mkdir(snapTarget, OPEN_DIR_MODE) == -1) {
      perror("mkdir-snap");
      exit(1);
  }
  static char command[200];
  sprintf(command, "rsync -av --progress . %s --exclude .vcs", snapTarget);
  system(command);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Wrong usage!\n");
    exit(1);
  }
  
  if (chdir(argv[1])) {
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
  return 0;
}
