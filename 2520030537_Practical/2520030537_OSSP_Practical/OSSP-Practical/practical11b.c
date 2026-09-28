#include <stdio.h>
#include <pthread.h>
#include <time.h>

#define THREADS 4
#define ITERATIONS 1000000

long long counter = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *increment(void *arg)
{
    (void)arg;

    for (int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&lock);

        counter++;

        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

int main(void)
{
    pthread_t threads[THREADS];

    struct timespec start, end;

    printf("PRACTICAL 11B: MUTEX SYNCHRONIZATION\n");

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < THREADS; i++) {
        if (pthread_create(&threads[i], NULL,
                           increment, NULL) != 0) {
            fprintf(stderr, "Thread creation failed.\n");
            return 1;
        }
    }

    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double seconds =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Expected counter: %d\n", THREADS * ITERATIONS);
    printf("Actual counter  : %lld\n", counter);
    printf("Execution time : %.6f seconds\n", seconds);

    pthread_mutex_destroy(&lock);

    return 0;
}
