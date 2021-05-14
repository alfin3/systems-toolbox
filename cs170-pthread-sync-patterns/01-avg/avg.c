/**
   avg.c

   A program for finding the average value of an array of random (double)
   numbers with multiple POSIX threads spawned from the main thread.

   usage        : ./avg count num_thds
   usage example: ./avg 100000000 3

   The example is adopted from 
   https://sites.cs.ucsb.edu/~rich/class/cs170/notes/ with added
   modifications, refactoring, and use of customized pthread and memory
   allocation utility functions in order to develop consistent naming
   and error checking conventions.

   A thread argument block is deallocated where it was previously allocated,
   consistent with the practice of deallocating where resources
   are allocated. However, the parent thread deallocates a result block 
   previously allocated by a child thread, consistent with 
   https://man7.org/linux/man-pages/man3/pthread_create.3.html
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "utilities-mem.h"
#include "utilities-pthread.h"

#define DRAND_SEED() do{srand48(time(NULL));}while (0)
#define DRAND() (drand48()) /* basic Linux random number generator */

const char *C_USAGE = "usage: ./avg count num_thds";

typedef struct{
  int id;
  int start;
  int count;
  double *data; /* pointer to parent data */
} thread_arg_t;

typedef struct{
  double sum;
} thread_res_t;

void *sum_thread(void *arg){
  int i;
  thread_arg_t *a = arg;
  thread_res_t *r = NULL;
  r = malloc_perror(1, sizeof(thread_res_t));
  r->sum = 0.0;
  printf("sum thread %d running, starting at %d for %d\n",
	 a->id,
	 a->start,
	 a->count);
  fflush(stdout);
  for (i = a->start; i < a->start + a->count; i++){
    r->sum += a->data[i];
  }
  printf("sum thread %d done, returning\n", a->id);
  fflush(stdout);
  return r;
}

int main(int argc, char **argv){
  int i;
  int count, seg_count, rem_count;
  int num_thds;
  int start = 0;
  double sum = 0.0;
  double *data = NULL; /* parent data block */
  pthread_t *sids = NULL;
  thread_arg_t *sa = NULL;
  thread_res_t *sr = NULL;

  /* input checking and initialization */
  DRAND_SEED();
  if (argc < 3){
    fprintf(stderr,"must specify count and number of threads\n%s\n",
	    C_USAGE);
    exit(EXIT_FAILURE);
  }
  count = atoi(argv[1]);
  num_thds = atoi(argv[2]);
  if (count < 1 || num_thds < 1 || num_thds > count){
    fprintf(stderr,"invalid input %d\n", count);
    exit(EXIT_FAILURE);
  }
  sids = malloc_perror(num_thds, sizeof(pthread_t));
  sa = malloc_perror(num_thds, sizeof(thread_arg_t));
  data = malloc_perror(count, sizeof(double));
  for (i = 0; i < count; i++){
    data[i] = DRAND();
  }
  seg_count = count / num_thds;
  rem_count = count % num_thds; /* to distribute among threads */

  /* spawn threads */
  printf("main thread about to create %d sum threads\n", num_thds);
  fflush(stdout);
  for (i = 0; i < num_thds; i++){
    sa[i].id = i;
    sa[i].count = seg_count;
    if (rem_count > 0){
      sa[i].count++;
      rem_count--;
    }
    sa[i].start = start;
    sa[i].data = data;
    printf("main thread creating sum thread %d\n", i);
    fflush(stdout);
    thread_create_perror(&sids[i], sum_thread, &sa[i]);
    printf("main thread has created sum thread %d\n", i);
    fflush(stdout);
    start += sa[i].count;
  }

  /* join with main */
  for (i = 0; i < num_thds; i++){
    printf("main thread about to join with sum thread %d\n", i);
    fflush(stdout);
    thread_join_perror(sids[i], (void **)&sr);
    printf("main thread joined with sum thread %d\n", i);
    fflush(stdout);
    sum += sr->sum;
    free(sr); /* result block allocated by a child thread */
    sr = NULL;
  }
  printf("the average over %d random numbers on [0.0 ,1.0) is %f\n",
	 count, (double)sum / count);
  free(sids);
  free(sa);
  free(data);
  sids = NULL;
  sa = NULL;
  data = NULL;
  return 0;
}

  
