#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <sys/wait.h>

#include "argparser.h"
#include "dynstr.h"

#define MAX_DIR_NAME 5000
#define OPEN_DIR_MODE (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define DIR_PREF ".snapshot_"

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

#define IS_ASCII(x) (x > 127)
bool syntacticalAnalysis(const char path[], off_t size) {
  static const char *DANGER_WORDS[] = {
    "danger", "risk", "warning"
  };
  const size_t DW_LEN = sizeof(DANGER_WORDS) / sizeof(void*);
  const int fd = getFD(path, O_RDONLY, 0);
  
  char *content = (char*)calloc(size, sizeof(char));
  if (read(fd, content, size) != size) {
    perror("read");
    free(content);
    exit(1);
  }

  for (size_t i = 0; i < DW_LEN; i++) {
    if (strstr(content, DANGER_WORDS[i])) {
      return false;
    }
  }

  int nrLines = 0;
  for (int i = 0; content[i]; i++) {
    nrLines += (content[i] == '\n');
  }
  //add nr lines cond

  for (int i = 0; content[i]; i++) {
    if (!IS_ASCII(content[i])) {
      return false;
    }
  }

  free(content);
  closeFD(fd);
  return true;
}

void iterDirRec(const char dirname[], String *json) {
  DIR *dir = openDir(dirname);
  for (struct dirent *deBuff; (deBuff = readdir(dir)) != NULL;) {
    char *d_name = deBuff->d_name;
    if (!strncmp(d_name, DIR_PREF, strlen(DIR_PREF)) || !strcmp(d_name, ".") || !strcmp(d_name, "..")) {
      continue;
    }
    struct stat statBuff;
    stat(d_name, &statBuff);
    if (S_ISDIR(statBuff.st_mode)) {
      append(json, d_name);
      append(json, ":[");
      iterDirRec(d_name, json);
      append(json, "],");
    } else {
      if (!hasRights(statBuff.st_mode)) {
        syntacticalAnalysis(d_name, statBuff.st_size);
      }
      char tmp[30];
      append(json, d_name);
      append(json, ":{size:");
      sprintf(tmp, "%ld", statBuff.st_size);
      append(json, tmp);
      append(json, ",last_modif:\"");
      append(json, ctime(&statBuff.st_mtime));
      pop(json);
      append(json, "\", inode_number:");
      sprintf(tmp, "%ld", statBuff.st_ino);
      append(json, tmp);
      append(json, "},");//*/
    }
  }
  pop(json);
  closeDir(dir);
}


#define TARGET_MAX_LEN 10000
void snapshot(const char targetDir[], const char pathToPut[]) {
  char snapTarget[TARGET_MAX_LEN] = "";
  strcat(snapTarget, pathToPut);
  strcat(snapTarget, "/");
  strcat(snapTarget, DIR_PREF);
  strcat(snapTarget, targetDir);
  strcat(snapTarget, "_");
  strcat(snapTarget, getCurrDateTime());
  if (mkdir(snapTarget, OPEN_DIR_MODE) == -1) {
      puts(snapTarget);
      perror("mkdir-snap");
      exit(1);
  }
  //copy
  static char command[TARGET_MAX_LEN * 2];
  sprintf(command, "rsync -av . %s --exclude .vcs", snapTarget);//[--progress]
  system(command);
  //save
  String json;
  initStr(&json);
  append(&json, "[");
  iterDirRec(".", &json);
  append(&json, "]");

  int fd = getFD(strcat(snapTarget, ".json"), O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
  write(fd, json.buffer, json.len);
  closeFD(fd);

  freeStr(&json);//*/
}

#define DIR_NAME_MAX 1024
void solve(const char dirname[], const char pathToPut[]) {
  if (chdir(dirname)) {
    perror("chdir");
    exit(1);
  }
  char targetDir[DIR_NAME_MAX];
  if (getcwd(targetDir, sizeof(targetDir)) == NULL) {
    perror("getcwd");
    exit(1);
  }
  snapshot(strrchr(targetDir, '/') + 1, pathToPut);//*/
}

void wrongUsage() {
  fprintf(stderr, "Wrong usage!\n");
  exit(1);
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    wrongUsage();
  }

  Args args;
  initArgs(argc, argv, &args);
  

  ArgPair *targets = getVal(&args, "-t");
  if (targets == NULL) {
    wrongUsage();
  }

  ArgPair *out = getVal(&args, "-o");

  const char *where = ".";
  if (out != NULL) {
    where = out->values[0];
  }

  for (int i = 0; i < targets->cnt; i++) {
      pid_t pid;
      if ((pid = fork()) < 0) {
        perror("fork");
        return 1;
      }
      if (pid == 0) {
        solve(targets->values[i], where);
        return 0;
      }
  }
  int status;
  pid_t pid;
  while ((pid = wait(&status)) > 0) {
    printf("Proccess %d exited with code %d.\n", pid, WEXITSTATUS(status));
  }
  return 0;
}
