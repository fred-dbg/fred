#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <sys/syscall.h>

#define NUM_THREADS 5

void printHeapAndStackLocations();
void sigint_handler();

static inline void memfence(){  asm volatile ("mfence" ::: "memory"); }

struct thread_arg {
  pthread_mutex_t *mutex;
  int id;
};

volatile int start = 0;

static int main_thread_private_data = 0;

void *worker(void *arg) {
  sleep(1);
}

/* Make this its own function so fredtest.py doesn't have to depend on
   line numbers. */
void print_solution()
{
  printf("Solution is: %d\n", 0);
}

int main() {
  pthread_t threads[NUM_THREADS+1];
  struct thread_arg targs[NUM_THREADS+1];
  pthread_mutex_t solution_mutex1 = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t solution_mutex2 = PTHREAD_MUTEX_INITIALIZER;
  int i = 0;
  int rc = 0;
  for (i = 1; i < 4; i++) {
    main_thread_private_data++;
    targs[i].id = i;
    targs[i].mutex = &solution_mutex1;
    rc = pthread_create(&threads[i], NULL, worker, (void *) &targs[i]);
    if (rc) {
      perror("pthread_create");
      return 57;
    }
  }
  for (i = 4; i < 6; i++) {
    main_thread_private_data++;
    targs[i].id = i;
    targs[i].mutex = &solution_mutex2;
    rc = pthread_create(&threads[i], NULL, worker, (void *) &targs[i]);
    if (rc) {
      perror("pthread_create");
      return 57;
    }
  }

  printf("Please checkpoint now.\n");
  main_thread_private_data++;
  start = 1;
  for (i = 1; i < NUM_THREADS+1; i++) {
    pthread_join(threads[i], NULL);
  }
  print_solution();

  exit(0);
}
