#ifndef __DYNSTR_H__
#define __DYNSTR_H__

typedef struct {
    char *buffer;
    int cap, len;
} String;

void initStr(String *string);
void append(String *string, const char s[]);
void freeStr(String *string);

#endif