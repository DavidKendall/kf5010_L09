/**
 * A monitor solution to the readers/writers problem
 * avoiding writer starvation
 */
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include "readers_writers.h"

static pthread_mutex_t monitorMutex;
static pthread_cond_t okToContinue;

static unsigned int ar = 0; /** number of active readers */
static unsigned int wr = 0; /** number of waiting readers */
static unsigned int aw = 0; /** number of active writers */
static unsigned int ww = 0; /** number of waiting writers */
static bool writerPriority = false; /** used to avoid writer starvation */

void rw_init(void) {
  pthread_mutex_init(&monitorMutex, NULL);
  pthread_cond_init(&okToContinue, NULL);
}

void rw_write(rw_func_t do_write, void *arg) {
  pthread_mutex_lock(&monitorMutex);
  while (ar + aw > 0) {
    ww += 1;
    pthread_cond_wait(&okToContinue, &monitorMutex);
    ww -= 1;
  }
  aw += 1;
  pthread_mutex_unlock(&monitorMutex);

  /* do write */
  do_write(arg);

  pthread_mutex_lock(&monitorMutex);
  aw -= 1;
  writerPriority = false;
  pthread_cond_broadcast(&okToContinue);
  pthread_mutex_unlock(&monitorMutex);
}


void rw_read(rw_func_t do_read, void *arg) {
  pthread_mutex_lock(&monitorMutex);
  while ((aw > 0) || ((ww > 0) && writerPriority)) {
    wr += 1;
    pthread_cond_wait(&okToContinue, &monitorMutex);
    wr -= 1;
  }
  ar += 1;
  pthread_mutex_unlock(&monitorMutex);

  /* do read */
  do_read(arg);

  pthread_mutex_lock(&monitorMutex);
  ar -= 1;
  writerPriority = true;
  if (ar == 0) {
    pthread_cond_broadcast(&okToContinue);
  }
  pthread_mutex_unlock(&monitorMutex);
}

