#include "argparser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void initArgs(int argc, char *argv[], Args *args) {
  int cnt = 0;
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      bool has = false;
      if (i + 1 < argc && argv[i + 1][0] != '-') {
        ArgPair *p = &(args->argPairs[cnt]);
        p->flag = argv[i];
        int j = 0;
        while (i + 1 < argc && argv[i + 1][0] != '-') {
          p->values[j] = argv[i + 1];
          i++, j++;
        }
        p->cnt = j;
        cnt++;
        has = true;
      }
      if (!has) {
        fprintf(stderr, "Missing arg!\n");
        exit(1);
      }
    }
  }
  args->cnt = cnt;
}

ArgPair *getVal(Args *const args, const char flag[]) {
  for (int i = 0; i < args->cnt; i++) {
    if (!strcmp(args->argPairs[i].flag, flag)) {
      return &args->argPairs[i];
    }
  }
  return NULL;
}
