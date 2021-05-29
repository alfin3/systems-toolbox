/**
   deadlock-free3.c
   
   A solution of the "Dining Philosophers" problem. In the provided
   implementation, a fifo queue is used to ensure (outside the scheduler)
   that the first thread to acquire mutex after calling state_pickup is
   also the first to "eat", guaranteeing non-starvation at the expense of
   performance. A thread may be able to "eat" but must wait for its turn.

   Correctness:

   Every thread pushed onto a queue is “thinking”. Thus, the first thread
   (head) in the queue can only be blocked by the threads that are “eating”
   and are not in the queue. The first thread will be popped.

   Thread starvation:

   The fifo queue guarantees that every waiting thread will make progress
   and will not starve. The queue is of fixed count because at
   most one instance of a thread id can be in the queue at any time.

   The functions for creating a thread synchronization state and pickup and
   putdown operations are called from the driver implemented in
   driver.c.

   The example is adopted from
   https://sites.cs.ucsb.edu/~rich/class/cs170/notes/, with added pthread and
   allocation utilities, modifications and fixes, in order to develop
   consistent naming and error checking conventions:
   -  state modifying functions take a state pointer and an integer id of
      a thread as parameters,
   -  a single while loop is used for waiting for the satisfaction of 
      the conjuction of three predicates (negated disjunction of negated
      predicates), each signaled separately, reducing code complexity
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

typedef enum{FALSE, TRUE} boolean_t;

typedef struct{
  int count; /* count - 1 is fixed count of the queue */
  int head;
  int tail;
  int *ids;
} queue_t;

typedef struct{
  int num_phil_threads;
  queue_t *queue;
  boolean_t *thinking;
  pthread_cond_t *cond_adj_thinking; /* signal to recheck predicates */
  pthread_mutex_t lock; /* modify the thinking array and thread id queue */
} thinking_t;

/**
   Initialize a fifo queue of thread ids with a fixed count.
*/
void queue_init(queue_t *q, int count){
  memset(q, 0, sizeof(queue_t)); /* head = 0 and tail = 0 */
  q->count = count + 1; /* + 1 due to fifo queue implementation */
  q->ids = malloc_perror(add_sz_perror(count, 1), sizeof(int));
}


/**
   Creates a new state for handling thread synchronization wrt
   pickup and putdown operations.
*/
void *state_new(int num_phil_threads){
  int i;
  thinking_t *ts = NULL;
  ts = malloc_perror(1, sizeof(thinking_t));
  ts->queue =  malloc_perror(1, sizeof(queue_t));
  ts->thinking =  malloc_perror(num_phil_threads, sizeof(boolean_t));
  ts->cond_adj_thinking =  malloc_perror(num_phil_threads,
					 sizeof(pthread_cond_t));
  ts->num_phil_threads = num_phil_threads;
  queue_init(ts->queue, num_phil_threads);
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
  int first;
  thinking_t *ts = state;
  mutex_lock_perror(&ts->lock);
  /* push thread id */
  ts->queue->tail = (ts->queue->tail + 1) % ts->queue->count;
  ts->queue->ids[ts->queue->tail] = id;
  /* wait for a) thread's turn and b) the adjacent threads to "think" */
  first = ts->queue->ids[(ts->queue->head + 1) % ts->queue->count];
  while (first != id ||
         !ts->thinking[(id + 1) % ts->num_phil_threads] ||
	 !ts->thinking[(id + ts->num_phil_threads - 1) %
		       ts->num_phil_threads]){
    cond_wait_perror(&ts->cond_adj_thinking[id], &ts->lock);
    first = ts->queue->ids[(ts->queue->head + 1) % ts->queue->count];
  }
  /* pop thread id */
  ts->queue->head = (ts->queue->head + 1) % ts->queue->count;
  ts->thinking[id] = FALSE;
  if (ts->queue->head != ts->queue->tail){
    /* non-empty queue; signal the new first thread to recheck predicates */
    first = ts->queue->ids[(ts->queue->head + 1) % ts->queue->count];
    cond_signal_perror(&ts->cond_adj_thinking[first]);
  }
  mutex_unlock_perror(&ts->lock);
}

/**
   Performs a putdown operation.
*/
void state_putdown(void *state, int id){
  thinking_t *ts = state;
  mutex_lock_perror(&ts->lock);
  ts->thinking[id] = TRUE;
  /* signal the adjacent threads to recheck predicates */
  cond_signal_perror(&ts->cond_adj_thinking[(id + 1) % ts->num_phil_threads]);
  cond_signal_perror(&ts->cond_adj_thinking[(id + ts->num_phil_threads - 1) %
					    ts->num_phil_threads]);
  mutex_unlock_perror(&ts->lock);
}
 
