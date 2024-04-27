#ifndef __OSUTILS_H__
#define __OSUTILS_H__

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

void getPipe(int[2]);
pid_t getForkPID();

#endif