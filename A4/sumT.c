// sumT.c - Array sum parallelized with pthreads (Assignment 3, Task 5 comparison)
//
// Compile: gcc -Wall -Wextra -O2 sumT.c -o sumT -pthread
// Run:     ./sumT <threads> [N]   (default N = 1,000,000,000)
//
// Changes from Assignment 2:
//  - any thread count (the table needs 5, 7 and 9)
//  - the last thread picks up the leftover elements when N doesn't divide evenly
//  - each thread sums into a local variable, then stores it once
//    (avoids false sharing on the partialSums array)
//  - the array is int (4 GB at N = 1e9) instead of long long (8 GB)
//  - timing with clock_gettime(CLOCK_MONOTONIC)

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Current time in seconds, using CLOCK_MONOTONIC as the assignment asks
static double now_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int *arr;               // shared input array
long long N;            // array size
long long *partialSums; // one partial sum per thread
int num_threads;

// Entry function for each thread: sum this thread's chunk of the array
void *sumPart(void *arg)
{
    int thread_id = *(int *)arg;

    long long chunk = N / num_threads;
    long long start = thread_id * chunk;
    long long end = (thread_id == num_threads - 1) ? N : start + chunk;

    long long local = 0;
    for (long long i = start; i < end; i++) {
        local += arr[i];
    }
    partialSums[thread_id] = local;

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <threads> [N]\n", argv[0]);
        return 1;
    }

    num_threads = atoi(argv[1]);
    N = (argc > 2) ? atoll(argv[2]) : 1000000000LL;

    if (num_threads < 1 || N <= 0) {
        printf("Threads and N must be positive.\n");
        return 1;
    }

    arr = malloc((size_t)N * sizeof(int));
    partialSums = calloc(num_threads, sizeof(long long));
    if (!arr || !partialSums) {
        perror("malloc");
        return 1;
    }

    // Initialize the array (not timed)
    for (long long i = 0; i < N; i++) {
        arr[i] = (int)(i + 1);
    }

    pthread_t threads[num_threads];
    int thread_ids[num_threads];

    double start = now_seconds();

    // Create threads to compute partial sums
    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, sumPart, &thread_ids[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Combine the partial sums
    long long totalSum = 0;
    for (int i = 0; i < num_threads; i++) {
        totalSum += partialSums[i];
    }

    double end = now_seconds();

    long long expected = N * (N + 1) / 2;
    printf("Pthread array sum, N = %lld, threads = %d\n", N, num_threads);
    printf("Total sum = %lld (%s)\n", totalSum, totalSum == expected ? "correct" : "WRONG");
    printf("Time taken: %f seconds\n", end - start);

    free(arr);
    free(partialSums);
    return 0;
}
