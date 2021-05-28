/**
   deadlock-free2.c
   
   A solution of the "Dining Philosophers" problem. In the provided
   implementation, a thread waits on the condition that both adjacent
   threads are "thinking" (condition variable for each thread). A
   deadlock cannot occur. Threads are treated according to the
   thread system and without an additional ordering scheme.

   The functions for creating a thread synchronization state and pickup and
   putdown operations are called from the driver implemented in
   driver.c.

   The example is adopted from
   https://sites.cs.ucsb.edu/~rich/class/cs170/notes/, with added pthread and
   allocation utilities, modifications and fixes, in order to develop
   consistent naming and error checking conventions:
   -  state modifying functions take a state pointer and an integer id of
      a thread as parameters,
   -  some variables are renamed or eliminated; some functions are
      renamed.
*/

#define _XOPEN_SOURCE 600

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "deadlock.h"
#include "utilities-mem.h"
#include "utilities-pthread.h"

typedef enum{FALSE, TRUE} boolean_t;

typedef struct{
  int num_phil_threads;
  boolean_t *thinking;
  pthread_cond_t *cond_adj_thinking; /* signal to recheck predicates */
  pthread_mutex_t lock; /* modify the thinking array */
} thinking_t;

/**
   Creates a new state for handling thread synchronization wrt
   pickup and putdown operations.
*/
void *state_new(int num_phil_threads){
  int i;
  thinking_t *ts = NULL;
  ts = malloc_perror(1, sizeof(thinking_t));
  ts->num_phil_threads = num_phil_threads;
  ts->thinking =  malloc_perror(num_phil_threads, sizeof(boolean_t));
  ts->cond_adj_thinking =  malloc_perror(num_phil_threads,
					 sizeof(pthread_cond_t));
  for (i = 0; i < ts->num_phil_threads; i++){
    ts->thinking[i] = TRUE;
    cond_init_perror(&ts->cond_adj_thinking[i]);
  }
  mutex_init_perror(&ts->lock);
  return ts;
}

/**
   Disposal of a state for thread synchronization, freeing memory resources
   is currently achieved with Ctrl+C while the driver is looping.
*/

/**
   Performs a pickup operation.
*/
void state_pickup(void *state, int id){
  thinking_t *ts = state;
  mutex_lock_perror(&ts->lock);
  while (!ts->thinking[(id + 1) % ts->num_phil_threads] ||
	 !ts->thinking[(id + ts->num_phil_threads - 1) %
		       ts->num_phil_threads]){
    cond_wait_perror(&ts->cond_adj_thinking[id], &ts->lock);
  }
  ts->thinking[id] = FALSE;
  mutex_unlock_perror(&ts->lock);
}

/**
   Performs a putdown operation.
*/
void state_putdown(void *state, int id){
  thinking_t *ts = state;
  mutex_lock_perror(&ts->lock);
  ts->thinking[id] = TRUE;
  cond_signal_perror(&ts->cond_adj_thinking[(id + 1) % ts->num_phil_threads]);
  cond_signal_perror(&ts->cond_adj_thinking[(id + ts->num_phil_threads - 1) %
					    ts->num_phil_threads]);
  mutex_unlock_perror(&ts->lock);
}
 
