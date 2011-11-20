#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define NUM_ITERS 1000
#define NUM_THREADS 8

static volatile unsigned start = 0;

void *thread_fnc(void *arg)
{
  unsigned i = 0;
  void *addr;
  while (!start) ;
  for (i = 0; i < NUM_ITERS; i++) {
    addr = malloc(1024*1024*10);
    assert(addr != NULL);
    free(addr);
  }
}

int main()
{
  pthread_t threads[NUM_THREADS];
  unsigned i = 0;

  for (i = 0; i < NUM_THREADS; i++) {
    pthread_create(&threads[i], NULL, thread_fnc, NULL);
  }

  start = 1;

  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  return 0;
}
