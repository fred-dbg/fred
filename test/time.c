#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>

#define NUM_THREADS 20
#define NUM_TIMES 1000

static void get_time()
{
  time_t t;
  struct tm *lt;
  t = time(NULL);
  lt = localtime(&t);
  printf("%s%ju secs since the Epoch\n", asctime(lt), (uintmax_t)t);
}

void *worker(void *arg)
{
  long i = 0;
  for (i = 0; i < NUM_TIMES; i++) {
    get_time();
    usleep(1);
  }
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
  }

  for (i = 0; i < NUM_THREADS; i++) {
    rc = pthread_join(threads[i], NULL);
    if (rc) {
      perror("pthread_join");
      return 1;
    }
  }

  return 0;
}
