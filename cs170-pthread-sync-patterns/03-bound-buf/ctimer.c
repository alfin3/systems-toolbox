#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

double ctimer(){
  struct timeval tm;
  gettimeofday(&tm, NULL);
  return tm.tv_sec + tm.tv_usec / (double)1000000;
}
