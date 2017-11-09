/*
 * main.c
 *
 * Bounded buffer test
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#include "buffer.h"

enum {
  N_PRODUCERS = 10,
  N_CONSUMERS = 10,
  N_ITERATIONS = 200,
  PRODUCER_DELAY_LB = 2000,
  PRODUCER_DELAY_UB = 10000,
  CONSUMER_DELAY_LB = 2000,
  CONSUMER_DELAY_UB = 10000,
  DATA_VALUE_LB = 0,
  DATA_VALUE_UB = 99,
};

static void *producer(void *arg);
static void *consumer(void *arg);
static void begin_threads(void);
static void end_threads(void);
static void random_init(long id, struct drand48_data *rand_state);
static void random_delay_producer(struct drand48_data *rand_state);
static void random_delay_consumer(struct drand48_data *rand_state);
static unsigned int random_value(struct drand48_data *rand_state);
static unsigned int random_int(struct drand48_data *rand_state, int l, int u);

static pthread_t thread[N_PRODUCERS + N_CONSUMERS];

int main(int argc, char *argv[]) {
  safe_buffer_init();
  begin_threads();
  end_threads();
  printf("\nFINISHED\n");
  return 0;
}

/**
 * @brief The function implementing the producer  behaviour.
 * @param arg identifies the producer: 0, 1, 2, ...
 */
static void *producer(void *arg) {

  struct drand48_data rand_state;
  message_t msg;

  msg.id = (long)(arg);
  random_init(msg.id, &rand_state);
  for (int n = 0; n < N_ITERATIONS; n+=1) {
    random_delay_producer(&rand_state);
    msg.data[0] = random_value(&rand_state);
    safe_buffer_put(&msg);
    safe_buffer_print();
  }
  pthread_exit(NULL);
}

/**
 * @brief The function implementing the consumer behaviour.
 * @param arg identifies the consumer: 0, 1, 2, ...
 */
static void *consumer(void *arg) {
  struct drand48_data rand_state;
  message_t msg;

  random_init((long)arg, &rand_state);
  for (int n = 0; n < N_ITERATIONS; n+=1) {
    random_delay_consumer(&rand_state);
    safe_buffer_get(&msg);
    safe_buffer_print();
  }
  pthread_exit(NULL);
}


static void begin_threads(void) {
  int rc;

  for (long i=0; i<N_PRODUCERS; i+=1) {
    rc = pthread_create(&thread[i], NULL,
			producer, (void *)i);
    assert(rc == 0);
  }
  for (long i=0; i<N_CONSUMERS; i+=1) {
    rc = pthread_create(&thread[N_PRODUCERS + i], NULL,
			consumer, (void *)i);
    assert(rc == 0);
  }
}


static void end_threads(void) {
  int rc;

  for (int i=0; i<N_PRODUCERS + N_CONSUMERS; i+=1) {
    rc = pthread_join(thread[i], NULL);
    assert(rc == 0);
  }
}

static void random_init(long id, struct drand48_data *rand_state) {
    int rc;

    /* initialise random seed */
    rc = srand48_r((long)id * 10 + time(NULL), rand_state);
    assert(rc == 0);
}

static void random_delay_producer(struct drand48_data *rand_state) {
  usleep(random_int(rand_state, PRODUCER_DELAY_LB, PRODUCER_DELAY_UB));
}

static void random_delay_consumer(struct drand48_data *rand_state) {
  usleep(random_int(rand_state, CONSUMER_DELAY_LB, CONSUMER_DELAY_UB));
}

static unsigned int random_value(struct drand48_data *rand_state) {
  return random_int(rand_state, DATA_VALUE_LB, DATA_VALUE_UB);
}

/**
 * @brief return a random integer in the range [l,u)
 * @param rand_state a pointer to a random state variable
 * @param l the lower bound of the range
 * @param u the upper bound of the range
 */
static unsigned int random_int(struct drand48_data *rand_state, int l, int u) {
    double r;
    int rc;

    assert((0 <= l) && (l < u));

    rc = drand48_r(rand_state, &r);
    assert(rc == 0);
    return (unsigned int)(lround(l + ((u - l) * r)));
}
