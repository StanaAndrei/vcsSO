#include "futils.h"

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
    perror("open");
    exit(1);
  }
  return fd;
}
 
void closeFD(int fd) {
  if (close(fd) < 0) {
    perror("close");
    exit(1);
  }
}

bool hasRights(mode_t perm) {
  bool ans = false;
  ans |= (perm & S_IRUSR);
  ans |= (perm & S_IWUSR);
  ans |= (perm & S_IXUSR);
  ans |= (perm & S_IRGRP);
  ans |= (perm & S_IWGRP);
  ans |= (perm & S_IXGRP);
  ans |= (perm & S_IROTH);
  ans |= (perm & S_IWOTH);
  ans |= (perm & S_IXOTH);
  return ans;
}
