/**
   race-condition-2.c

   usage        : ./race-cond2 num_thds len iter
   usage example: ./race-cond2 4 100 5

   The example is adopted from
   https://sites.cs.ucsb.edu/~rich/class/cs170/notes/, with added pthread
   and allocation utilities, modifications and fixes, in order to develop
   consistent naming and error checking conventions.
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "utilities-mem.h"
#include "utilities-pthread.h"

const int C_PREEMPTION_PROB_ITER = 800000;
const char *C_USAGE = "usage: ./race-cond2 num_thds len iter";

typedef struct{
  char *s;
  int id;
  int len;
  int iter;
} print_arg_t;

void *print_thread(void *arg){
  int i, j, k;
  print_arg_t *a = arg;
  for (i = 0; i < a->iter; i++){
    for (j = 0; j < a->len; j++){
      a->s[j] = 'A'+ a->id;
      /* increase the probability of preemption within a string */
      for(k = 0; k < C_PREEMPTION_PROB_ITER; k++); 
    }
    a->s[a->len] = '\0';
    printf("thread %d: %s\n", a->id, a->s);
  }
  return NULL;
}

int main(int argc, char **argv)
{
  char *s = NULL;
  int i, num_thds;
  int len, iter;
  pthread_t *pids = NULL;
  print_arg_t *pas = NULL;
  if (argc != 4){
    fprintf(stderr,"%s\n", C_USAGE);
    exit(EXIT_FAILURE);
  }

  /* initialize */
  num_thds = atoi(argv[1]);
  len = atoi(argv[2]);
  iter = atoi(argv[3]);
  s = malloc_perror(add_sz_perror(len, 1), sizeof(char));
  pids = malloc_perror(num_thds, sizeof(pthread_t));
  pas = malloc_perror(num_thds, sizeof(print_arg_t));
  
  /* spawn threads */
  for (i = 0; i < num_thds; i++){
    pas[i].id = i;
    pas[i].len = len;
    pas[i].iter = iter;
    pas[i].s = s; /* pointer to the parent string */
    thread_create_perror(&pids[i], print_thread, &pas[i]);
  }

  /* join with main */
  for (i = 0; i < num_thds; i++){
    thread_join_perror(pids[i], NULL);
  }
  free(s);
  free(pids);
  free(pas);
  s = NULL;
  pids = NULL;
  pas = NULL;
  return 0;
}
