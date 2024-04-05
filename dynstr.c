#include "dynstr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initStr(String *string) {
    string->buffer = NULL;
    string->cap = string->len = 0;
}

void append(String *string, const char s[]) {
    int len = strlen(s);
    int newLen = string->len + len;

    if (string->cap <= newLen) {
        string->cap = newLen + 1;
        string->buffer = (char*)realloc(string->buffer, string->cap);
        if (string->buffer == NULL) {
            perror("alloc");
            exit(1);
        }
    }

    strcat(string->buffer, s);
    string->len = newLen;
}

void freeStr(String *string) {
    free(string->buffer);
    string->buffer = NULL;
    string->cap = string->len = 0;
}