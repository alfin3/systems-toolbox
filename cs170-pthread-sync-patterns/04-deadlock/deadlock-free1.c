/**
   deadlock-free1.c
   
   A solution of the "Dining Philosophers" problem. In the
   provided implementation threads with odd ids acquire the first mutex
   on the left (odd, left first), and threads with even ids acquire the
   first mutex on the right (even, right first). The pairs
   (odd, right first) and (even, left first) are also correct by symmetry.
   A deadlock cannot occur.

   Correctness (# threads > 1):

   At any time, there exists a thread A that will acquire or has acquired
   its first mutex.
   case 1: A is odd and the adjacent threads are even, or A is even and
           the adjacent threads are odd. If A waits for the second mutex,
           then an adjacent thread has acquired this mutex as its second
           mutex and will release it. Thus, A will acquire its second 
           mutex, unless the other thread reacquires it. No deadlock.
   case 2: A is even and an adjacent thread B is even.
           a) The first mutex of A is the second mutex of B. The second
              mutex of A is the second mutex of another thread. Thus, A
              will acquire its second mutex, unless the other thread
              reacquires it. No deadlock.
           b) The second mutex of A is the first mutex of B. If A waits
              for its second mutex, then B acquired it. The second mutex
              of B is the second mutex of another thread. Thus, B will
              release its first mutex, unless the other thread requires
              B's second mutex. If B releases its first mutex, A will 
              acquire its second mutex, unless B requires it.
              No deadlock.

   Fairness:

   The solution is prone to unfair treatment of threads. In the setting
   with five threads, thread 4 has an unfair advantage over other threads
   and is blocked for less time, because its first mutex is the second
   mutex of thread 0, whereas for all other threads the first mutex is
   also the first mutex of another thread. A second mutex has a smaller
   critical section than a first mutex. As A competes for its
   first mutex, the competing thread spends less time locking it.

   The probability of preemption is increased by calling sleep after
   acquiring the first mutex in state_pickup, and as a result block
   times are higher as compared to deadlock1.

   The functions for creating a thread synchronization state and pickup and
   putdown operations are called from the driver implemented in
   driver.c.

   The example is adopted from
   https://sites.cs.ucsb.edu/~rich/class/cs170/notes/, with added correctness
   proof, pthread and allocation utilities, modifications and fixes, in order
   to develop consistent naming and error checking conventions:
   -  state modifying functions take a state pointer and an integer id of
      a thread as parameters,
   -  some variables are renamed or eliminated; some functions are
      renamed,
   -  a correctness proof is provided.
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
  if (id & 1){
    /* lock the left fork and then the right fork */
    mutex_lock_perror(&fs->locks[id]);
    sleep(C_INTERLOCK_TIME);
    mutex_lock_perror(&fs->locks[(id + 1) % fs->num_phil_threads]);
  }else{
    /* lock the right fork and then the left fork */
    mutex_lock_perror(&fs->locks[(id + 1) % fs->num_phil_threads]);
    sleep(C_INTERLOCK_TIME);
    mutex_lock_perror(&fs->locks[id]);
  }
}

/**
   Performs a putdown operation.
*/
void state_putdown(void *state, int id){
  forks_t *fs = state;
  if (id & 1){
    /* unlock the right fork and then the left fork */
    mutex_unlock_perror(&fs->locks[(id + 1) % fs->num_phil_threads]);
    mutex_unlock_perror(&fs->locks[id]);
  }else{
    /* unlock the left fork and then the right fork */
    mutex_unlock_perror(&fs->locks[id]);
    mutex_unlock_perror(&fs->locks[(id + 1) % fs->num_phil_threads]);
  }
}
 
