/**
   utilities-pthread.h

   Declarations of accessible utility functions for concurrency, including
   1) pthread functions with wrapped error checking, and
   2) an implementation of semaphore operations based on 1),
   adopted from The Little Book of Semaphores by Allen B. Downey
   (Version 2.2.1) with modifications.
*/

#ifndef UTILITIES_PTHREAD_H
#define UTILITIES_PTHREAD_H

#include <pthread.h>

typedef struct{
  int value;
  unsigned int num_wakeups;
  pthread_mutex_t mutex; /* the result of referring to a copy is undefined */
  pthread_cond_t cond; /* the result of referring to a copy is undefined */
} sema_t; /* the result of referring to a copy of an instance is undefined */


/**
   Create a thread with default attributes and error checking. Join a thread
   with error checking.
*/
void thread_create_perror(pthread_t *thread,
			  void *(*start_routine)(void *),
			  void *arg);

void thread_join_perror(pthread_t thread, void **retval);

/**
   Initialize with default attributes, lock, and unlock a mutex with
   error checking.
*/

void mutex_init_perror(pthread_mutex_t *mutex);

void mutex_lock_perror(pthread_mutex_t *mutex);

void mutex_unlock_perror(pthread_mutex_t *mutex);

/**
   Initialize a condition variable with default attributes and
   error checking. Wait on and signal a condition with error checking.
*/

void cond_init_perror(pthread_cond_t *cond);

void cond_wait_perror(pthread_cond_t *cond, pthread_mutex_t *mutex);

void cond_signal_perror(pthread_cond_t *cond);

/**
   Initialize, wait on, and signal a semaphore with error checking
   provided by mutex and condition variable operations.
*/

void sema_init_perror(sema_t *sema, int value);

void sema_wait_perror(sema_t *sema);

void sema_signal_perror(sema_t *sema);

#endif
