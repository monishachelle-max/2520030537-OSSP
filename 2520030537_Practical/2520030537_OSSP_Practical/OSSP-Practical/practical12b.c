#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t lock_a = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_b = PTHREAD_MUTEX_INITIALIZER;

pthread_barrier_t barrier;
int prevention = 0;

void *thread_one(void *arg)
{
    (void)arg;

    pthread_mutex_lock(&lock_a);
    printf("Thread 1 acquired Resource A\n");
    fflush(stdout);

    if (!prevention)
        pthread_barrier_wait(&barrier);

    pthread_mutex_lock(&lock_b);
    printf("Thread 1 acquired Resource B\n");

    pthread_mutex_unlock(&lock_b);
    pthread_mutex_unlock(&lock_a);

    printf("Thread 1 completed\n");
    return NULL;
}

void *thread_two(void *arg)
{
    (void)arg;

    if (prevention) {
        /* Both threads acquire resources in the same order. */
        pthread_mutex_lock(&lock_a);
        printf("Thread 2 acquired Resource A\n");

        pthread_mutex_lock(&lock_b);
        printf("Thread 2 acquired Resource B\n");

        pthread_mutex_unlock(&lock_b);
        pthread_mutex_unlock(&lock_a);

        printf("Thread 2 completed\n");
    } else {
        /* Opposite order creates a deadlock. */
        pthread_mutex_lock(&lock_b);
        printf("Thread 2 acquired Resource B\n");
        fflush(stdout);

        pthread_barrier_wait(&barrier);

        pthread_mutex_lock(&lock_a);
        printf("Thread 2 acquired Resource A\n");

        pthread_mutex_unlock(&lock_a);
        pthread_mutex_unlock(&lock_b);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2 ||
        (strcmp(argv[1], "deadlock") != 0 &&
         strcmp(argv[1], "prevent") != 0)) {
        fprintf(stderr,
                "Usage: %s deadlock|prevent\n", argv[0]);
        return 1;
    }

    prevention = strcmp(argv[1], "prevent") == 0;

    pthread_t t1, t2;

    if (!prevention)
        pthread_barrier_init(&barrier, NULL, 2);

    printf("PRACTICAL 12B - %s\n",
           prevention ? "DEADLOCK PREVENTION" :
                        "DEADLOCK DEMONSTRATION");
    fflush(stdout);

    pthread_create(&t1, NULL, thread_one, NULL);
    pthread_create(&t2, NULL, thread_two, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    if (!prevention)
        pthread_barrier_destroy(&barrier);

    pthread_mutex_destroy(&lock_a);
    pthread_mutex_destroy(&lock_b);

    printf("Both threads completed successfully.\n");

    return 0;
}
