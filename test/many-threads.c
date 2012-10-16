#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 500

void *worker(void *arg)
{
  long i = (long)arg;
  printf("Thread %ld alive\n", i);
  usleep(100);
  printf("aaaaand thread %ld exiting.\n", i);
}

int main()
{
  pthread_t threads[NUM_THREADS];
  long i = 0;
  int rc = 0;
  for (i = 0; i < NUM_THREADS; i++) {
    rc = pthread_create(&threads[i], NULL, worker, (void *)i);
    if (rc) {
      perror("pthread_create");
      return 1;
    }
    rc = pthread_join(threads[i], NULL);
    if (rc) {
      perror("pthread_join");
      return 1;
    }
  }

  return 0;
}
