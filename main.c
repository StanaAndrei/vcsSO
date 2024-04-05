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
      char tmp[30];
      append(json, d_name);
      append(json, ":{size:");
      sprintf(tmp, "%ld", statBuff.st_size);
      append(json, tmp);
      append(json, ",last_modif:\"");
      append(json, ctime(&statBuff.st_mtime));
      pop(json);
      append(json, "\"},");//*/
    }
  }
  pop(json);
  closeDir(dir);
}

void snapshot(const char pathToPut[]) {
  char snapTarget[100] = "";
  strcat(snapTarget, pathToPut);
  strcat(snapTarget, "/");
  strcat(snapTarget, DIR_PREF);
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
  String json;
  initStr(&json);
  append(&json, "[");
  iterDirRec(".", &json);
  append(&json, "]");

  int fd = getFD(strcat(snapTarget, ".json"), O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
  write(fd, json.buffer, json.len);
  closeFD(fd);

  freeStr(&json);
}

void solve(const char dirname[], const char pathToPut[]) {
  if (chdir(dirname)) {
    perror("chdir");
    exit(1);
  }

  snapshot(pathToPut);
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

  if (out == NULL) {
    for (int i = 0; i < targets->cnt; i++) {
      solve(targets->values[i], ".");
    }//*/
  } else {
    for (int i = 0; i < targets->cnt; i++) {
      solve(targets->values[i], out->values[0]);
    }//*/
  }

  return 0;
}
