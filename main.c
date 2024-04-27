#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>

#include "argparser.h"
#include "dynstr.h"
#include "futils.h"
#include "defs.h"
#include "defs.h"
#include "timeutils.h"
#include "osutils.h"

bool syntacticalAnalysis(const char path[]) {
  const char *const pathCopy = strdup(path);
  if (!pathCopy) {
    perror("strdup");
    exit(1);
  }

  int pipefd[2];
  getPipe(pipefd);

  const char *argv[] = { SCRIPT_NAME, pathCopy, NULL };
  const char *envp[] = { NULL };
  pid_t pid = getForkPID();
  if (pid == 0) {
    closeFD(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    closeFD(pipefd[1]);
    execve(SCRIPT_NAME, (char *const *)argv, (char *const *)envp);
    perror("execve");
    exit(1);
  }

  closeFD(pipefd[1]);
  char buffer[MAX_FILE_NAME];
  ssize_t bytesRead = readHelper(pipefd[0], buffer, MAX_FILE_NAME);
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

void iterDirRec(const char dirname[], String *json, const char safeSpace[], int *potDanger) {
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
      iterDirRec(pathTo, json, safeSpace, potDanger);
      append(json, "],");
    } else {
      if (!hasRights(statBuff.st_mode)) {
        if (!syntacticalAnalysis(pathTo)) {
          (*potDanger)++;
          //moveFile(pathTo, tmp, statBuff.st_size);
          char command[strlen(pathTo) + strlen(safeSpace) + 10];
          sprintf(command, "mv %s %s", pathTo, safeSpace);
          system(command);
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

int snapshot(const char targetDir[], const char pathToPut[], const char safeSpace[]) {
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
  int potDanger = 0;
  iterDirRec(targetDir, &json, safeSpace, &potDanger);
  append(&json, "]\n");

  int fd = getFD(strcat(snapTarget, ".json"), O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
  writeHelper(fd, json.buffer, json.len);
  closeFD(fd);

  freeStr(&json);//*/
  return potDanger;
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
  ArgPair *safeSpaceArg = getVal(&args, "-s");
  if (targets == NULL || safeSpaceArg == NULL) {
    wrongUsage();
  }

  const int numProcs = targets->cnt;
  int pipefd[numProcs][2];
  for (int i = 0; i < numProcs; i++) {
    getPipe(pipefd[i]);
  }

  ArgPair *out = getVal(&args, "-o");
  const char *where = ((out == NULL) ? NULL : out->values[0]);
  for (int i = 0; i < numProcs; i++) {
      pid_t pid = getForkPID();
      if (pid == 0) {
        int potDanger = snapshot(targets->values[i], where, safeSpaceArg->values[0]);
        closeFD(pipefd[i][0]);
        writeHelper(pipefd[i][1], &potDanger, sizeof(potDanger));
        closeFD(pipefd[i][1]);
        return 0;
      }
  }

  for (int i = 0, cnt = 1; i < numProcs; i++, cnt++) {
    pid_t pid;
    int status;
    closeFD(pipefd[i][1]);
    int potDanger = -1;
    readHelper(pipefd[i][0], &potDanger, sizeof(potDanger));
    closeFD(pipefd[i][0]);
    if ((pid = wait(&status)) > 0) {
      const char format[] = "Proccess %d with pid %d exited with code %d and %d potentially dangerous files.\n";
      printf(format, cnt, pid, WEXITSTATUS(status), potDanger);
    }
  }
  return 0;
}
