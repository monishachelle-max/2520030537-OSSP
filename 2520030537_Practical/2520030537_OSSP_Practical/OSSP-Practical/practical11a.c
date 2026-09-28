#include <stdio.h>
#include <pthread.h>

#define THREADS 4
#define ITERATIONS 1000000

long long counter = 0;

void *increment(void *arg)
{
    (void)arg;

    for (int i = 0; i < ITERATIONS; i++) {
        counter++;
    }

    return NULL;
}

int main(void)
{
    pthread_t threads[THREADS];

    printf("PRACTICAL 11A: RACE CONDITION\n");

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

    printf("Expected counter: %d\n", THREADS * ITERATIONS);
    printf("Actual counter  : %lld\n", counter);

    return 0;
}
