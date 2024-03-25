#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#define MAX_DIR_NAME 50
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

void snapshot(const char path[]) {
  DIR *dir = openDir(path);
  for (struct dirent *deBuff; (deBuff = readdir(dir)) != NULL;) {
    if (strcmp(deBuff->d_name, ".vcs") == 0) {
      continue;
    }
    puts(deBuff->d_name);
  }
  closeDir(dir);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Wrong usage!\n");
    exit(1);
  }
  
  static char target[MAX_DIR_NAME];
  strcpy(target, argv[1]);
  strcat(target, "/.vcs");

  DIR *vcsDir;
  if ((vcsDir = opendir(target)) == NULL && errno == ENOENT) {
    if (mkdir(target, OPEN_DIR_MODE) == -1) {
      perror("mkdir");
      exit(1);
    }
  }

  snapshot(argv[1]);
  closeDir(vcsDir);
  return 0;
}
