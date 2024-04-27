#include "osutils.h"

void getPipe(int pipefd[2]) {
  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(1);
  }
}

pid_t getForkPID() {
    pid_t pid;
    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    }
    return pid;
}