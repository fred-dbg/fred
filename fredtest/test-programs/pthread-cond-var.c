#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_WORKER_THREADS 15 
#define CV_HITS_LIMIT (NUM_WORKER_THREADS * 1000)

static pthread_cond_t *cv = NULL;
static pthread_mutex_t *mutex = NULL;

static volatile long solution = 0;
static volatile int resources_available = 0;
static volatile int cv_hits = 0;

static void print_solution() {
    printf("Solution is: %ld\n", solution);
}

static void initialize_cond_objects()
{
   cv = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
   if (cv == NULL) {
      perror("malloc");
      exit(1);
   }

   mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
   if (mutex == NULL) {
      perror("malloc");
      exit(1);
   }

   pthread_cond_init(cv, NULL);
   pthread_mutex_init(mutex, NULL);
}

static void destroy_cond_objects()
{
   pthread_mutex_destroy(mutex);
   pthread_cond_destroy(cv);

   free(mutex);
   free(cv);
}

static void *worker(void *arg)
{
    long id = (long)arg;
    printf("Starting watcher %ld.\n", id);
    while (1) {
        pthread_mutex_lock(mutex);
        while (resources_available == 0) {
            pthread_cond_wait(cv, mutex);
        }
        if (cv_hits > CV_HITS_LIMIT) {
            printf("!!! Watcher %ld exiting.\n", id);
            pthread_mutex_unlock(mutex);
            pthread_exit(NULL);
        }
        cv_hits++;
        solution += id;
        resources_available--;
        pthread_mutex_unlock(mutex);
    }
}

static void *resource_generator(void *arg)
{
    printf("Starting resource generator thread.\n");
    while (1) {
        pthread_mutex_lock(mutex);
        if (cv_hits > CV_HITS_LIMIT) {
            printf("!!! Resource generator thread exiting.\n");
            pthread_mutex_unlock(mutex);
            pthread_exit(NULL);
        }
        resources_available += 2;
        pthread_cond_broadcast(cv);
        pthread_mutex_unlock(mutex);
    }
}

int main()
{
    pthread_t threads[NUM_WORKER_THREADS+1];
    long i = 0;
    int rc = 0;

    initialize_cond_objects();

    for (i = 0; i < NUM_WORKER_THREADS; i++) {
        rc = pthread_create(&threads[i], NULL, worker, (void *)i);
        if (rc) {
            perror("pthread_create");
            return 1;
        }
    }
    rc = pthread_create(&threads[NUM_WORKER_THREADS], NULL,
                        resource_generator, NULL);
    if (rc) {
        perror("pthread_create");
        return 1;
    }

    for (i = 0; i < NUM_WORKER_THREADS + 1; i++) {
        rc = pthread_join(threads[i], NULL);
        if (rc) {
            perror("pthread_join");
            return 1;
        }
    }
    
    destroy_cond_objects();
    print_solution();
    return 0;
}
