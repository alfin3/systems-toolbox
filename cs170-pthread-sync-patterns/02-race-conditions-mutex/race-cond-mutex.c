/**
   race-condition-mutex.c

   usage        : ./race-cond-mutex num_threads len iter
   usage example: ./race-cond-mutex 4 100 5

   The example is adopted from
   https://sites.cs.ucsb.edu/~rich/class/cs170/notes/, with added pthread
   and allocation utilities, modifications and fixes, in order to develop
   consistent naming and error checking conventions.
*/

#define _XOPEN_SOURCE 600

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "utilities-mem.h"
#include "utilities-pthread.h"

const int C_PREEMPTION_PROB_ITER = 800000;
const char *C_USAGE = "usage: ./race-cond-mutex num_threads len iter";

typedef struct{
  char *s;
  int id;
  int len;
  int iter;
  pthread_mutex_t *lock_str; /* pointer to a single shared mutex */
} print_arg_t;

void *print_thread(void *arg){
  int i, j, k;
  print_arg_t *a = arg;
  for (i = 0; i < a->iter; i++){
    mutex_lock_perror(a->lock_str);
    for (j = 0; j < a->len; j++){
      a->s[j] = 'A'+ a->id;
      /* increase the probability of preemption within a string */
      for(k = 0; k < C_PREEMPTION_PROB_ITER; k++); 
    }
    a->s[a->len] = '\0';
    printf("thread %d: %s\n", a->id, a->s);
    mutex_unlock_perror(a->lock_str);
  }
  return NULL;
}

int main(int argc, char **argv){
  char *s = NULL;
  int i, num_threads;
  int len, iter, err;
  pthread_mutex_t lock_str; /* shared among all print threads */
  pthread_t *pids = NULL;
  pthread_attr_t *patts = NULL;
  print_arg_t *pas = NULL;
  if (argc != 4){
    fprintf(stderr,"%s\n", C_USAGE);
    exit(EXIT_FAILURE);
  }

  /* initialize */
  num_threads = atoi(argv[1]);
  len = atoi(argv[2]);
  iter = atoi(argv[3]);
  mutex_init_perror(&lock_str);
  s = malloc_perror(add_sz_perror(len, 1), sizeof(char));
  pids = malloc_perror(num_threads, sizeof(pthread_t));
  patts = malloc_perror(num_threads, sizeof(pthread_attr_t));
  pas = malloc_perror(num_threads, sizeof(print_arg_t));
  
  /* spawn threads */
  for (i = 0; i < num_threads; i++){
    pas[i].s = s; /* pointer to the parent string */
    pas[i].id = i;
    pas[i].len = len;
    pas[i].iter = iter;
    pas[i].lock_str = &lock_str;
    pthread_attr_init(&patts[i]);
    pthread_attr_setscope(&patts[i], PTHREAD_SCOPE_SYSTEM);
    err = pthread_create(&pids[i], &patts[i], print_thread, &pas[i]);
    if (err != 0){
      perror("pthread_create failed");
      exit(EXIT_FAILURE);
    }
  }

  /* join with main */
  for (i = 0; i < num_threads; i++){
    thread_join_perror(pids[i], NULL);
  }
  free(s);
  free(pids);
  free(patts);
  free(pas);
  s = NULL;
  pids = NULL;
  patts = NULL;
  pas = NULL;
  return 0;
}
