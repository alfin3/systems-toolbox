/**
   deadlock2.c

   A solution of the "Dining Philosophers" problem. The implementation
   is prone to deadlock, which is likely to be reached due to an increased
   probability of preemption resulting from a call to sleep after acquiring
   the first mutex and before acquiring the second mutex in state_pickup.
   
   The functions for creating a thread synchronization state and pickup and
   putdown operations are called from the driver implemented in
   driver.c.

   The example is adopted from
   https://sites.cs.ucsb.edu/~rich/class/cs170/notes/, with added pthread
   and allocation utilities, modifications and fixes, in order to develop
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

const long C_INTERLOCK_TIME = 3;

typedef struct{
  int num_phil_threads;
  pthread_mutex_t *locks;
} forks_t;

/**
   Creates a new state for handling thread synchronization wrt
   pickup and putdown operations.
*/
void *state_new(int num_phil_threads){
  int i;
  forks_t *fs = NULL;
  fs = malloc_perror(1, sizeof(forks_t));
  fs->num_phil_threads = num_phil_threads;
  fs->locks =  malloc_perror(num_phil_threads, sizeof(pthread_mutex_t));
  for (i = 0; i < fs->num_phil_threads; i++){
    mutex_init_perror(&fs->locks[i]);
  }
  return fs;
}

/**
   Disposal of a state for thread synchronization, freeing memory resources
   is currently achieved with Ctrl+C while the driver is looping.
*/

/**
   Performs a pickup operation.
*/
void state_pickup(void *state, int id){
  forks_t *fs = state;
  /* lock the left fork and then the right fork */
  mutex_lock_perror(&fs->locks[id]);
  sleep(C_INTERLOCK_TIME);
  mutex_lock_perror(&fs->locks[(id + 1) % fs->num_phil_threads]);
}

/**
   Performs a putdown operation.
*/
void state_putdown(void *state, int id){
  forks_t *fs = state;
  /* unlock the right fork and then the left fork */
  mutex_unlock_perror(&fs->locks[(id + 1) % fs->num_phil_threads]);
  mutex_unlock_perror(&fs->locks[id]);
}
