#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>

#define ITEMS 100000
#define PRODUCERS 2
#define CONSUMERS 2

int *buffer;
int buffer_size;
int in = 0, out = 0;

sem_t empty_slots;
sem_t full_slots;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

long long total_sum = 0;
int consumed_count = 0;

double elapsed(struct timespec a, struct timespec b)
{
    return (b.tv_sec - a.tv_sec) +
           (b.tv_nsec - a.tv_nsec) / 1e9;
}

void wait_sem(sem_t *sem)
{
    while (sem_wait(sem) == -1) {
        if (errno != EINTR) {
            perror("sem_wait");
            exit(1);
        }
    }
}

void *producer(void *arg)
{
    int id = *(int *)arg;
    int start = id * (ITEMS / PRODUCERS) + 1;
    int end = (id + 1) * (ITEMS / PRODUCERS);

    for (int value = start; value <= end; value++) {
        wait_sem(&empty_slots);

        pthread_mutex_lock(&mutex);

        buffer[in] = value;
        in = (in + 1) % buffer_size;

        pthread_mutex_unlock(&mutex);

        sem_post(&full_slots);
    }

    return NULL;
}

void *consumer(void *arg)
{
    (void)arg;

    for (int i = 0; i < ITEMS / CONSUMERS; i++) {
        wait_sem(&full_slots);

        pthread_mutex_lock(&mutex);

        int value = buffer[out];
        out = (out + 1) % buffer_size;

        total_sum += value;
        consumed_count++;

        pthread_mutex_unlock(&mutex);

        sem_post(&empty_slots);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s BUFFER_SIZE\n", argv[0]);
        return 1;
    }

    buffer_size = atoi(argv[1]);

    if (buffer_size <= 0 || buffer_size > 100000) {
        fprintf(stderr, "Buffer size must be 1 to 100000.\n");
        return 1;
    }

    buffer = malloc((size_t)buffer_size * sizeof(int));

    if (buffer == NULL) {
        perror("malloc");
        return 1;
    }

    if (sem_init(&empty_slots, 0, (unsigned int)buffer_size) == -1 ||
        sem_init(&full_slots, 0, 0) == -1) {
        perror("sem_init");
        free(buffer);
        return 1;
    }

    pthread_t producers[PRODUCERS];
    pthread_t consumers[CONSUMERS];
    int ids[PRODUCERS];

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < CONSUMERS; i++)
        pthread_create(&consumers[i], NULL, consumer, NULL);

    for (int i = 0; i < PRODUCERS; i++) {
        ids[i] = i;
        pthread_create(&producers[i], NULL, producer, &ids[i]);
    }

    for (int i = 0; i < PRODUCERS; i++)
        pthread_join(producers[i], NULL);

    for (int i = 0; i < CONSUMERS; i++)
        pthread_join(consumers[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double seconds = elapsed(start, end);
    long long expected_sum =
        (long long)ITEMS * (ITEMS + 1) / 2;

    printf("PRACTICAL 12A - PRODUCER CONSUMER\n");
    printf("Buffer size: %d\n", buffer_size);
    printf("Produced: %d\n", ITEMS);
    printf("Consumed: %d\n", consumed_count);
    printf("Expected sum: %lld\n", expected_sum);
    printf("Actual sum: %lld\n", total_sum);
    printf("Execution time: %.6f seconds\n", seconds);
    printf("Throughput: %.2f items/second\n",
           seconds > 0 ? ITEMS / seconds : 0.0);

    if (consumed_count == ITEMS && total_sum == expected_sum)
        printf("Synchronization: PASS\n");
    else
        printf("Synchronization: FAIL\n");

    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);
    pthread_mutex_destroy(&mutex);
    free(buffer);

    return 0;
}
