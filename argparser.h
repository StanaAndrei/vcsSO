#ifndef __ARGPARSER_H__
#define __ARGPARSER_H__

#define MAX_FLAG_VALUES 15
#define MAX_ARGS 10
 

typedef struct {
  char *flag;
  char* values[MAX_FLAG_VALUES];
  int cnt;
} ArgPair;

typedef struct {
  ArgPair argPairs[MAX_ARGS];
  int cnt;
} Args;

void initArgs(int argc, char *argv[], Args *args);
ArgPair *getVal(Args *const args, const char flag[]);

#endif
