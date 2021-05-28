/**
   deadlock.h

   Declarations of functions for running solutions of the "Dining 
   Philosophers" problem.

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

#ifndef DEADLOCK_H
#define DEADLOCK_H

/**
   Creates a new state for handling thread synchronization wrt
   pickup and putdown operations.
*/
void *state_new(int num_phil_threads);

/**
   Disposal of a state for thread synchronization, freeing memory resources
   is currently achieved with Ctrl+C while the driver is looping.
*/

/**
   Performs a pickup operation.
*/
void state_pickup(void *state, int id);

/**
   Performs a putdown operation.
*/
void state_putdown(void *state, int id);

#define NUM_THREADS_MIN (2) /* used as int */
#define NUM_THREADS_MAX (25) /* used as int */

#endif
