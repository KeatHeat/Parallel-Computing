#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX_THREADS 12
#define ITERATIONS 100000000
#define CACHE_LINE 64

typedef struct {
    int id;
    int num_threads;
    volatile double *sum;
} ThreadData;

typedef struct {
    volatile double sum;
    char unused[CACHE_LINE - sizeof(double)];
} Separated;


static double get_time(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (double)ts.tv_sec +
           (double)ts.tv_nsec / 1000000000.0;
}


static void *worker(void *arg)
{
    ThreadData *data = (ThreadData *)arg;

    // double local = 0.0;

    double value;

    switch (data->id % 4) {
        case 0:
            value = 1e11;
            break;

        case 1:
            value = -1e11;
            break;

        // case 2:
        //     value = 1e-18;
        //     break;

        default:
            value = 1e-6;
            break;
    }

    for (size_t i = 0; i < ITERATIONS; i++) {
        // local += value;
        // *data->sum = local;
        *data->sum += value;
    }

    return NULL;
}


static void run_a(int threads)
{
    pthread_t tid[MAX_THREADS];
    ThreadData data[MAX_THREADS];

    volatile double sums[MAX_THREADS] = {0};

    double start = get_time();

    for (int i = 0; i < threads; i++) {

        data[i].id = i;
        data[i].num_threads = threads;
        data[i].sum = &sums[i];

        if (pthread_create(
                &tid[i],
                NULL,
                worker,
                &data[i]) != 0) {

            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < threads; i++)
        pthread_join(tid[i], NULL);

    double elapsed = get_time() - start;

    double forward = 0.0;

    for (int i = 0; i < threads; i++)
        forward += sums[i];

    double reverse = 0.0;

    for (int i = threads - 1; i >= 0; i--)
        reverse += sums[i];

    printf(
        "A  threads=%2d  time=%8.4f  "
        "forward=%-22.17g  "
        "reverse=%-22.17g  "
        "difference=%g\n",
        threads,
        elapsed,
        forward,
        reverse,
        forward - reverse);
}


static void run_b(int threads)
{
    pthread_t tid[MAX_THREADS];
    ThreadData data[MAX_THREADS];

    Separated sums[MAX_THREADS] = {0};

    double start = get_time();

    for (int i = 0; i < threads; i++) {

        data[i].id = i;
        data[i].num_threads = threads;
        data[i].sum = &sums[i].sum;

        if (pthread_create(
                &tid[i],
                NULL,
                worker,
                &data[i]) != 0) {

            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < threads; i++)
        pthread_join(tid[i], NULL);

    double elapsed = get_time() - start;

    double forward = 0.0;

    for (int i = 0; i < threads; i++)
        forward += sums[i].sum;

    double reverse = 0.0;

    for (int i = threads - 1; i >= 0; i--)
        reverse += sums[i].sum;

    printf(
        "B  threads=%2d  time=%8.4f  "
        "forward=%-22.17g  "
        "reverse=%-22.17g  "
        "difference=%g\n",
        threads,
        elapsed,
        forward,
        reverse,
        forward - reverse);
}


int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: %s <threads>\n", argv[0]);
        return 1;
    }

    int threads = atoi(argv[1]);

    if (threads < 1 || threads > MAX_THREADS) {
        printf(
            "Threads must be between 1 and %d\n",
            MAX_THREADS);

        return 1;
    }

    printf("\n");
    printf("Threads: %d\n", threads);
    printf("Iterations: %d\n\n", ITERATIONS);

    run_a(threads);
    run_b(threads);

    return 0;
}
