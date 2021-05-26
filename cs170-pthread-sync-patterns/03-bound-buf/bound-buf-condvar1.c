/**
   bound-buf-condvar1.c

   A program for running a bounded buffer (producer-consumer) example
   by using mutex locks and condition variables, the latter to reduce 
   while loop polling in time slices. Requires x86.

   usage example on a 4-core machine:
   ./bound-buf-condvar1 -c 3 -t 1 -q 1 -s 100 -o 1000000
   ./bound-buf-condvar1 -c 3 -t 1 -q 2 -s 100 -o 1000000
   ./bound-buf-condvar1 -c 3 -t 1 -q 3 -s 100 -o 1000000
   ./bound-buf-condvar1 -c 3 -t 1 -q 4 -s 100 -o 1000000
   ./bound-buf-condvar1 -c 1 -t 1 -q 3 -s 100 -o 1000000
   ./bound-buf-condvar1 -c 1 -t 3 -q 3 -s 100 -o 1000000
   ./bound-buf-condvar1 -c 2 -t 2 -q 3 -s 10 -o 3 -V

   The example is adopted from
   https://sites.cs.ucsb.edu/~rich/class/cs170/notes/, with added pthread
   and allocation utilities, modifications and fixes, in order to develop
   consistent naming and error checking conventions:
   -  queue and market initialization and freeing functions are changed
      to require a pointer to a preallocated block,
   -  the names of some variables are changed and a few minor bugs
      are fixed; the names of condition variables are changed to negated
      names to reflect their use,
   -  the outer polling while loops are removed due to the use of
      pthread_cond_wait within dedicated predicate re-testing while loops.
*/

#define _XOPEN_SOURCE 600

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include "ctimer.h"
#include "utilities-mem.h"
#include "utilities-pthread.h"

#define DRAND_SEED() do{srand48(time(NULL));}while (0)
#define DRAND() (drand48()) /* basic Linux random number generator */
#define ARGS "c:t:o:q:s:V"

typedef enum{FALSE, TRUE} boolean_t;
typedef enum{BUY, SELL} action_t;

const int C_DEF_NUM_CLIENT_THREADS = 1;
const int C_DEF_NUM_TRADER_THREADS = 1;
const int C_DEF_ORDERS_PER_CLIENT = 1;
const int C_DEF_QUEUE_COUNT = 1;
const int C_DEF_NUM_STOCKS = 1;
const int C_DEF_QUANTITY = 5000;
const double C_PROB_HALF = 0.5; 

const char *C_USAGE =
  "bound-buf-mutex "
  "-c clients "
  "-t traders "
  "-o orders "
  "-q queue-count "
  "-s number-stocks"
  "-V <verbose on>\n";

/**
   Order, order queue structs, as well as initialization and freeing 
   functions.
*/

typedef struct{
  int stock_id;
  int quantity;
  action_t action;
  boolean_t fulfilled;	
} order_t;

typedef struct{
  int count; /* count - 1 is fixed count of the queue -> bounded buffer */
  int head;
  int tail;
  order_t **orders;
  pthread_mutex_t lock;
  pthread_cond_t cond_nfull;
  pthread_cond_t cond_nempty;
} order_q_t;

void order_q_init(order_q_t *q, int count){
  memset(q, 0, sizeof(order_q_t)); /* head = 0 and tail = 0 */
  q->count = count + 1; /* + 1 due to fifo queue implementation */
  q->orders = calloc_perror(q->count, sizeof(order_t *));
  mutex_init_perror(&q->lock);
  cond_init_perror(&q->cond_nfull);
  cond_init_perror(&q->cond_nempty);
}

void order_q_free(order_q_t *q){
  while (q->head != q->tail){
    q->head = (q->head + 1) % q->count;
    free(q->orders[q->head]);
    q->orders[q->head] = NULL;
  }
  free(q->orders);
  q->orders = NULL;
}

/**
   Market struct, as well as initialization and freeing functions. 
*/

typedef struct market{
  int num_stocks;
  int *quantities;
  pthread_mutex_t lock;
} market_t;

void market_init(market_t *m, int num_stocks, int quantity){
  int i;
  m->num_stocks = num_stocks;
  m->quantities = malloc_perror(num_stocks, sizeof(int));
  for (i = 0; i < num_stocks; i++){
    m->quantities[i] = quantity;
  }
  mutex_init_perror(&m->lock);
}

void market_free(market_t *m){
  free(m->quantities);
  m->quantities = NULL;
}

void market_print(market_t *m){
  int i;
  for(i = 0; i < m->num_stocks; i++){
    printf("stock: %d, quantity: %d\n", i , m->quantities[i]);
  }
}

/**
   Client (producer) and trader (consumer) thread arguments and entry 
   functions.
*/

typedef struct{
  int id;
  int order_count;
  int num_stocks;
  int quantity;
  boolean_t verbose;
  order_q_t *q; /* clients (producers) and traders (consumers) */
} client_arg_t;

typedef struct{
  int id;
  boolean_t *done;
  boolean_t verbose;
  order_q_t *q; /* clients (producers) and traders (consumers) */
  market_t *m; /* only traders (consumers) */
} trader_arg_t;

/**
   Produces and queues order_count orders. After queuing an order, loops 
   (waits) until the order is fulfilled before queuing the next order.
*/
void *client_thread(void *arg){
  int i;
  int next;
  order_t *order = NULL;
  client_arg_t *ca = arg;
  order = malloc_perror(1, sizeof(order_t));
  for (i = 0; i < ca->order_count; i++){
    /* produce an order */
    order->stock_id = DRAND() * (ca->num_stocks - 1);
    order->quantity = DRAND() * ca->quantity;
    order->action = (DRAND() > C_PROB_HALF) ? BUY : SELL;
    order->fulfilled = FALSE;
    /* queue the order */
    mutex_lock_perror(&ca->q->lock);
    next = (ca->q->tail + 1) % ca->q->count;
    while (next == ca->q->head){
      /* queue is full; wait for cond_nfull signal and retest
         because "at least one" waiting thread is unblocked */
      cond_wait_perror(&ca->q->cond_nfull, &ca->q->lock);
      next = (ca->q->tail + 1) % ca->q->count;
    }
    /* queue is not full; queue the order and unlock mutex */
    if (ca->verbose){
      printf("%10.6f client %d: ", ctimer(), ca->id);
      printf("queued stock %d, for %d, %s\n",
	     order->stock_id,
	     order->quantity,
	     (order->action ? "SELL" : "BUY"));
    }
    ca->q->orders[next] = order;
    ca->q->tail = next;
    cond_signal_perror(&ca->q->cond_nempty);
    mutex_unlock_perror(&ca->q->lock);
    /* wait until fulfilled; atomic read in x86 */
    while (!order->fulfilled);
  }
  free(order);
  order = NULL;
  return NULL;
}

/**
   Dequeues and consumes orders, as long as there are orders.
*/
void *trader_thread(void *arg){
  int next;
  order_t *order = NULL;
  trader_arg_t *ta = arg;
  while (TRUE){
    /* dequeue or exit if done */
    mutex_lock_perror(&ta->q->lock);
    while (ta->q->head == ta->q->tail){
      if (*ta->done){
	cond_signal_perror(&ta->q->cond_nempty);
	mutex_unlock_perror(&ta->q->lock);
	return NULL;
      }
      /* after the last order is processed, all trader threads may be 
         blocked; need to signal cont_nempty after done is set to TRUE */
      cond_wait_perror(&ta->q->cond_nempty, &ta->q->lock);
    }
    next = (ta->q->head + 1) % ta->q->count;
    order = ta->q->orders[next];
    ta->q->head = next;
    cond_signal_perror(&ta->q->cond_nfull);
    mutex_unlock_perror(&ta->q->lock);
    /* process a dequeued order */
    mutex_lock_perror(&ta->m->lock);
    if (order->action == BUY){
      ta->m->quantities[order->stock_id] -= order->quantity;
      if (ta->m->quantities[order->stock_id] < 0){
	ta->m->quantities[order->stock_id] = 0;
      }
    }else{
      ta->m->quantities[order->stock_id] += order->quantity;
    }
    if (ta->verbose){
      printf("%10.6f trader: %d ", ctimer(), ta->id);
      printf("fulfilled stock %d for %d\n",
	     order->stock_id,
	     order->quantity);
    }
    mutex_unlock_perror(&ta->m->lock);
    /* atomic memory write on x86; inform the reading client thread */
    order->fulfilled = TRUE;
  }
}

int main(int argc, char **argv){
  int i;
  int num_client_threads = C_DEF_NUM_CLIENT_THREADS;
  int num_trader_threads = C_DEF_NUM_TRADER_THREADS;
  int orders_per_client = C_DEF_ORDERS_PER_CLIENT;
  int queue_count = C_DEF_QUEUE_COUNT;
  int num_stocks = C_DEF_NUM_STOCKS;
  int quantity = C_DEF_QUANTITY;
  int c;
  double start, end;
  boolean_t verbose = FALSE;
  boolean_t done = FALSE;
  order_q_t *q = NULL;
  market_t *m = NULL;
  pthread_t *cids = NULL;
  pthread_t *tids = NULL;
  client_arg_t *cas = NULL;
  trader_arg_t *tas = NULL;
  DRAND_SEED();
  while ((c = getopt(argc, argv, ARGS)) != -1){
    switch (c){
    case 'c':
      num_client_threads = atoi(optarg);
      if (num_client_threads < 1){
	fprintf(stderr,"number of client threads must be > 0\n");
	exit(EXIT_FAILURE);
      }
      break;
    case 't':
      num_trader_threads = atoi(optarg);
      if (num_trader_threads < 1){
	fprintf(stderr,"number of trader threads must be > 0\n");
	exit(EXIT_FAILURE);
      }
      break;
    case 'o':
      orders_per_client = atoi(optarg);
      if (orders_per_client < 0){
	fprintf(stderr,"orders per client must be non-negative\n");
	exit(EXIT_FAILURE);
      }
      break;
    case 'q':
      queue_count = atoi(optarg);
      if (queue_count < 1 || queue_count > INT_MAX - 1){
	fprintf(stderr,"invalid queue count\n");
	exit(EXIT_FAILURE);
      }
      break;
    case 's':
      num_stocks = atoi(optarg);
      if (num_stocks < 1){
	fprintf(stderr,"number of stocks must be > 0\n");
	exit(EXIT_FAILURE);
      }
      break;
    case 'V':
      verbose = TRUE;
      break;
    default:
      fprintf(stderr, "unrecognized command %c\n", (char)c);
      fprintf(stderr,"usage: %s\n", C_USAGE);
      exit(EXIT_FAILURE);
    }
  }
  q = malloc_perror(1, sizeof(order_q_t));
  m = malloc_perror(1, sizeof(market_t));
  cids = malloc_perror(num_client_threads, sizeof(pthread_t));
  tids = malloc_perror(num_trader_threads, sizeof(pthread_t));
  cas = malloc_perror(num_client_threads, sizeof(client_arg_t));
  tas = malloc_perror(num_trader_threads, sizeof(trader_arg_t));
  order_q_init(q, queue_count);
  market_init(m, num_stocks, quantity);
  start = ctimer();
  /* spawn threads */
  for (i = 0; i < num_client_threads; i++){
    cas[i].id = i;
    cas[i].order_count = orders_per_client;
    cas[i].num_stocks = num_stocks;
    cas[i].quantity = quantity;
    cas[i].q = q;
    cas[i].verbose = verbose;
    thread_create_perror(&cids[i], client_thread, &cas[i]);
  }
  for (i = 0; i < num_trader_threads; i++){
    tas[i].id = i;
    tas[i].q = q;
    tas[i].m = m;
    tas[i].done = &done;
    tas[i].verbose = verbose;
    thread_create_perror(&tids[i], trader_thread, &tas[i]);
  }
  /* join client threads after each client's orders are fulfilled */
  for (i = 0; i < num_client_threads; i++){
    thread_join_perror(cids[i], NULL);
  }
  /* signal cond_nempty because all trader threads may be blocked */
  mutex_lock_perror(&q->lock);
  done = TRUE;
  cond_signal_perror(&q->cond_nempty);
  mutex_unlock_perror(&q->lock);
  for (i = 0; i < num_trader_threads; i++){
    thread_join_perror(tids[i], NULL);
  }
  end = ctimer();
  if (verbose) market_print(m);
  printf("%f transactions / sec\n",
	 orders_per_client * num_client_threads / (end - start));
  order_q_free(q);
  market_free(m);
  free(q);
  free(m);
  free(cids);
  free(tids);
  free(cas);
  free(tas);
  q = NULL;
  m = NULL;
  cids = NULL;
  tids = NULL;
  cas = NULL;
  tas = NULL;
  return 0;
}
