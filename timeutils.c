#include "timeutils.h"

char *getCurrDateTime() {
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  static char timeStr[100]; 
  sprintf(timeStr, "%d-%02d-%02d_%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  return timeStr;
}