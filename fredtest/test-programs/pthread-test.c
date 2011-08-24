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

volatile long long solution = 0;
volatile int lock_counter = 0;
volatile int start = 0;
__thread int should_exit = 0;
pthread_mutex_t lock_counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void sig_handler(int signum) {
  exit(0);
}

void *worker(void *arg) {
  struct thread_arg *targ = (struct thread_arg *)arg;
  pthread_mutex_t *mutex = targ->mutex;
  int id = targ->id;
  int i = 0;
  while (!start) ;
  while (1) {
    pthread_mutex_lock(mutex);
    pthread_mutex_lock(&lock_counter_mutex);
    if (lock_counter > 100) {
      should_exit = 1;
    }
    printf("************ lock_counter: %d ***********\n", lock_counter);
    fflush(stdout);
    lock_counter++;
    pthread_mutex_unlock(&lock_counter_mutex);
    solution += id;
    pthread_mutex_unlock(mutex);
    //printf("************** two ***************\n");
    fflush(stdout);
    if (should_exit == 1) {
      printf("!!! Thread %d exiting.\n", id);
      pthread_exit(NULL);
    }
  }
}

int main() {
  pthread_t threads[NUM_THREADS+1];
  struct thread_arg targs[NUM_THREADS+1];
  pthread_mutex_t solution_mutex1 = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t solution_mutex2 = PTHREAD_MUTEX_INITIALIZER;
  int i = 0;
  int rc = 0;
  signal(SIGINT, sig_handler);
  for (i = 1; i < 4; i++) {
    targs[i].id = i;
    targs[i].mutex = &solution_mutex1;
    rc = pthread_create(&threads[i], NULL, worker, (void *) &targs[i]);
    if (rc) {
      perror("pthread_create");
      return 57;
    }
  }
  for (i = 4; i < 6; i++) {
    targs[i].id = i;
    targs[i].mutex = &solution_mutex2;
    rc = pthread_create(&threads[i], NULL, worker, (void *) &targs[i]);
    if (rc) {
      perror("pthread_create");
      return 57;
    }
  }

  printf("Please checkpoint now.\n");
  start = 1;
  for (i = 1; i < NUM_THREADS+1; i++) {
    pthread_join(threads[i], NULL);
  }
  printf("Solution is: %lld\n", solution);

  exit(0);
}
