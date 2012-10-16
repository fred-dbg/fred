#include <stdio.h>
#include <pthread.h>

/*
 * Creates CONCURRENT_THREADS number of threads which are alive at
 * once. Does this a total of TOTAL_NUM_THREADS / CONCURRENT_THREADS
 * times.
 */

#define TOTAL_NUM_THREADS 1000
#define CONCURRENT_THREADS 20
#define THREAD_LIFETIME_US 100000
//#define PRINTING

void *worker(void *arg)
{
  long i = (long)arg;
#ifdef PRINTING
  printf("Thread %ld alive\n", i);
#endif
  usleep(THREAD_LIFETIME_US);
#ifdef PRINTING
  printf("aaaaand thread %ld exiting.\n", i);
#endif
}

int main()
{
  pthread_t threads[CONCURRENT_THREADS];
  long i = 0;
  int rc = 0;
  for (i = 0; i < TOTAL_NUM_THREADS; i += CONCURRENT_THREADS) {
    long j;
    for (j = 0; j < CONCURRENT_THREADS; j++) {
      rc = pthread_create(&threads[j], NULL, worker, (void *)(i+j));
      if (rc) {
        perror("pthread_create");
        return 1;
      }
    }
    for (j = 0; j < CONCURRENT_THREADS; j++) {
      rc = pthread_join(threads[j], NULL);
      if (rc) {
        perror("pthread_join");
        return 1;
      }
    }
  }

  return 0;
}
