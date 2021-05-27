/**
   driver.c

   Driver functions for solutions of the "Dining Philosophers" problem.
   
   usage: ./executable num_phil_threads max_dur
   usage example: deadlock1 5 5

   The example is adopted from
   https://sites.cs.ucsb.edu/~rich/class/cs170/notes/, with added pthread
   and allocation utilities, modifications and fixes, in order to develop
   consistent naming and error checking conventions:
   -  state modifying functions take a state pointer and an integer id of
      a thread as parameters; the thread argument struct is moved to
      driver.c, where the thread entry function is defined,
   -  some variables are renamed or eliminated; some functions are
      renamed.
*/

#define _XOPEN_SOURCE 600

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "deadlock.h"
#include "utilities-mem.h"
#include "utilities-pthread.h"

#define RANDOM_SEED() do{srandom(time(NULL));}while (0)
#define RANDOM() (random()) /* basic Linux random number generator */

typedef enum{FALSE, TRUE} boolean_t;

const char *C_USAGE = "./executable num_phil_threads max_dur";

typedef struct{
  int id;
  int num_phil_threads;
  long start_time;
  long max_dur; /* max time for thinking/eating */
  long *block_times; /* total time each thread is blocked */
  void *state; /* synchronization state wrt pickup and putdown ops */
  pthread_mutex_t *lock_block_times; /* updating and printing */
} phil_arg_t;

void *phil_thread(void *arg){
  long t;
  phil_arg_t *pa = arg;
  while (TRUE){
    /* think */
    t = RANDOM() % (pa->max_dur + 1);
    printf("%3ld Philosopher %d thinking for %ld seconds\n", 
                time(NULL) - pa->start_time, pa->id, t);
    fflush(stdout);
    sleep(t);
    /* pick up */
    printf("%3ld Philosopher %d calling state_pickup\n", 
            time(NULL) - pa->start_time, pa->id);
    fflush(stdout);
    t = time(NULL);
    state_pickup(pa->state, pa->id);
    mutex_lock_perror(pa->lock_block_times);
    pa->block_times[pa->id] += time(NULL) - t;
    mutex_unlock_perror(pa->lock_block_times);
    /* eat */
    t = RANDOM() % (pa->max_dur + 1);
    printf("%3ld Philosopher %d eating for %ld seconds\n", 
                time(NULL) - pa->start_time, pa->id, t);
    fflush(stdout);
    sleep(t);
    /* put down */
    printf("%3ld Philosopher %d calling state_putdown\n", 
            time(NULL) - pa->start_time, pa->id);
    fflush(stdout);
    state_putdown(pa->state, pa->id);
  }
}

int main(int argc, char **argv){
  char s[BUF_SIZE_MAX];
  char *cur = NULL;
  int i, num_phil_threads;
  int print_interval = 10;
  long max_dur;
  long start_time = time(NULL);
  long *block_times = NULL;
  long total_block_time = 0;
  void *state = NULL;
  pthread_t pids[NUM_THREADS_MAX];
  phil_arg_t pas[NUM_THREADS_MAX];
  pthread_mutex_t lock_block_times;
  RANDOM_SEED();
  if (argc != 3) {
    fprintf(stderr, "usage: %s\n", C_USAGE);
    exit(EXIT_FAILURE);
  }
  num_phil_threads = atoi(argv[1]);
  max_dur = atoi(argv[2]);
  if (num_phil_threads > NUM_THREADS_MAX || num_phil_threads < 1){
    fprintf(stderr, "number of threads must be positive and > %d\n",
	    NUM_THREADS_MAX);
    exit(EXIT_FAILURE);
  }
  if (max_dur < 0){
    fprintf(stderr, "maximal eating/thinking duration must be "
	    "nonnegative\n");
    exit(EXIT_FAILURE);
  }
  block_times = calloc_perror(num_phil_threads, sizeof(long));
  state = state_new(num_phil_threads);
  mutex_init_perror(&lock_block_times);
  for (i = 0; i < num_phil_threads; i++){
    pas[i].id = i;
    pas[i].start_time = start_time;;
    pas[i].max_dur = max_dur;
    pas[i].block_times = block_times;
    pas[i].state = state;
    pas[i].lock_block_times = &lock_block_times;
    thread_create_perror(&pids[i], phil_thread, &pas[i]);
  }
  while (TRUE){
    /* exit and free resources with Ctrl+C */
    mutex_lock_perror(&lock_block_times);
    cur = s;
    for(i = 0; i < num_phil_threads; i++){
      total_block_time += block_times[i];
    }
    sprintf(cur,"%3ld Total blocktime: %5ld : ",
	    time(NULL) - start_time, total_block_time);
    cur = s + strlen(s);
    for(i = 0; i < num_phil_threads; i++){
    	sprintf(cur, "%5ld ", block_times[i]);
	cur = s + strlen(s);
    }
    mutex_unlock_perror(&lock_block_times);
    printf("%s\n", s);
    fflush(stdout);
    sleep(print_interval);
  }
  /* no dellocation performed due to Ctrl+C */
}
