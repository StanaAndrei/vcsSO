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

#define MAX_FILE_NAME (1 << 8)
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

#define SAFE_STR "SAFE"
#define SCRIPT_NAME "verify_for_malicious.sh"
bool syntacticalAnalysis(const char path[]) {
  const char *const pathCopy = strdup(path);
  if (!pathCopy) {
    perror("strdup");
    exit(1);
  }

  int pipefd[2];
  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(1);
  }

  const char *argv[] = { SCRIPT_NAME, pathCopy, NULL };
  const char *envp[] = { NULL };
  pid_t pid;
  if ((pid = fork()) < 0) {
    perror("fork");
    exit(1);
  }
  if (pid == 0) {
    closeFD(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);
    execve(SCRIPT_NAME, (char *const *)argv, (char *const *)envp);
    perror("execve");
    exit(1);
  }

  closeFD(pipefd[1]);
  char buffer[MAX_FILE_NAME];
  ssize_t bytesRead;
  if ((bytesRead = read(pipefd[0], buffer, MAX_FILE_NAME)) == EOF) {
    perror("read");
    exit(1);
  }
  buffer[bytesRead - 1] = 0;
  closeFD(pipefd[0]);
  int status;
  waitpid(pid, &status, 0);
  printf("Script (pid: %d) finished with status %d.\n", pid, WEXITSTATUS(status));
  if (status) {
    exit(WEXITSTATUS(status));
  }
  return !strcmp(buffer, SAFE_STR);
}

void iterDirRec(const char dirname[], String *json) {
  DIR *dir = openDir(dirname);
  for (struct dirent *deBuff; (deBuff = readdir(dir)) != NULL;) {
    const char *d_name = deBuff->d_name;
    if (!strncmp(d_name, DIR_PREF, strlen(DIR_PREF)) || !strcmp(d_name, ".") || !strcmp(d_name, "..")) {
      continue;
    }
    struct stat statBuff;
    char pathTo[strlen(dirname) + strlen(d_name) + 3];
    *pathTo = 0;
    strcat(pathTo, dirname);
    strcat(pathTo, d_name);
    if (stat(pathTo, &statBuff) == -1) {
      perror("stat");
      exit(1);
    }
    if (S_ISDIR(statBuff.st_mode)) {
      append(json, d_name);
      append(json, ":[");
      strcat(pathTo, "/");
      iterDirRec(pathTo, json);
      append(json, "],");
    } else {
      if (!hasRights(statBuff.st_mode)) {
        if (!syntacticalAnalysis(pathTo)) {
          printf("danger %s!!\n", pathTo);
          continue;
        }
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
  if (pathToPut == NULL) {
    strcat(snapTarget, targetDir);
    strcat(snapTarget, DIR_PREF);
    strcat(snapTarget, getCurrDateTime());
  } else {
    strcat(snapTarget, pathToPut);
    strcat(snapTarget, DIR_PREF);
    strcat(snapTarget, getCurrDateTime());
  }
  puts(snapTarget);
  String json;
  initStr(&json);
  append(&json, "[");
  iterDirRec(targetDir, &json);
  append(&json, "]\n");

  int fd = getFD(strcat(snapTarget, ".json"), O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
  write(fd, json.buffer, json.len);
  closeFD(fd);

  freeStr(&json);//*/
}

#define DIR_NAME_MAX 1024
void solve(const char dirname[], const char pathToPut[]) {
  snapshot(dirname, pathToPut);//*/
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

  const char *where = NULL;
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
