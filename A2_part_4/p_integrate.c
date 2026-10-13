
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define N 1000000000 // intervals

// Function being integrated: 4 / (1 + x^2) from 0 to 1 equals pi
double f(double x) {
    return 4.0 / (1.0 + x * x);
}

//use this shared global volume
double total_sum = 0.0;
pthread_mutex_t sum_mutex = PTHREAD_MUTEX_INITIALIZER;


// Sequential trapezoidal rule (baseline for benchmarking)
double sequential_trapezoidalRule(void)
{
    double h = 1.0 / N;
    double sum = 0.0;
    for (long i = 0; i < N; i++) {
        double x1 = i * h;
        double x2 = (i + 1) * h;
        sum += (f(x1) + f(x2)) * h / 2.0;
    }
    return sum;
}

// Each thread integrates its own block of intervals into a local sum,
// then adds it to the shared total once (mutex protects total_sum).
void *parallel_trapezoidalRule(void *arg)
{
    // Thread ID and thread count are packed into one number: id * 10 + count
    long thread_data = (long)arg;
    long thread_id = thread_data / 10;
    long num_threads = thread_data % 10;

    // Find this thread's range of intervals (last thread takes any leftover)
    long start = thread_id * (N / num_threads);
    long end = (thread_id == num_threads - 1)
        ? N
        : (thread_id + 1) * (N / num_threads);

    double h = 1.0 / N;
    double local_sum = 0.0;

    // Add up the area of each trapezoid in this thread's range
    for (long i = start; i < end; i++) {
        double x1 = i * h;
        double x2 = (i + 1) * h;

        local_sum += (f(x1) + f(x2)) * h / 2.0;
    }

    // Only one thread can update the shared total at a time
    pthread_mutex_lock(&sum_mutex);
    total_sum += local_sum;
    pthread_mutex_unlock(&sum_mutex);

    return NULL;
}


int main(int argc, char *argv[]){
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_threads>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);

    if (num_threads < 1 || num_threads > 4) {
        fprintf(stderr, "Thread count must be between 1 and 4.\n");
        return 1;
    }

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));

    if (threads == NULL) {
        perror("malloc");
        return 1;
    }
    
    struct timespec start_time, end_time;

    // ----- sequential baseline -----
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    double seq_result = sequential_trapezoidalRule();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double seq_elapsed = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    printf("Sequential result: %.10f\n", seq_result);
    printf("Sequential time: %.6f seconds\n", seq_elapsed);

    // ----- parallel version -----
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // Create the threads and give each one its ID and the thread count
    for (long i = 0; i < num_threads; i++) {
        long thread_data = i * 10 + num_threads;
        if (pthread_create(&threads[i], NULL, parallel_trapezoidalRule, (void *)thread_data) != 0) {
            perror("pthread_create");
            free(threads);
            return 1;
        }
    }


    // Wait for all threads to finish before reading total_sum
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    double elapsed = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

    printf("Result of numerical integration: %.10f\n", total_sum);
    printf("Threads: %d\n", num_threads);
    printf("Execution time: %.6f seconds\n", elapsed);

    pthread_mutex_destroy(&sum_mutex);
    free(threads);

    return 0;
}
