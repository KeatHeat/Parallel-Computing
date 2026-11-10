// nqueensT.c - N Queens parallelized with pthreads
//
// Compile: gcc -Wall -Wextra -O2 nqueensT.c -o nqueensT -pthread
// Run:     ./nqueensT <threads> [N]   (default N = 15)
//
// The search is split into n*n tasks (first two queens fixed). Thread k
// takes tasks k, k + T, k + 2T, ... (round robin), counts solutions in
// its own board, and stores its total in partialCounts[k].

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
#include "nqueens.h"

int n;
int num_threads;
long long *partialCounts;

void *queensPart(void *arg)
{
    int id = *(int *)arg;
    long long local = 0;

    for (int t = id; t < n * n; t += num_threads) {
        local += solve_task(n, t);
    }
    partialCounts[id] = local;
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <threads> [N]\n", argv[0]);
        return 1;
    }

    num_threads = atoi(argv[1]);
    n = (argc > 2) ? atoi(argv[2]) : 15;
    if (num_threads < 1 || n < 2 || n > MAXN) {
        printf("Threads must be >= 1 and N between 2 and %d.\n", MAXN);
        return 1;
    }

    partialCounts = calloc(num_threads, sizeof(long long));
    pthread_t threads[num_threads];
    int ids[num_threads];

    double start = now_seconds();

    // Create the threads
    for (int i = 0; i < num_threads; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, queensPart, &ids[i]);
    }

    // Wait for them and add up their counts
    long long count = 0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        count += partialCounts[i];
    }

    double end = now_seconds();

    printf("Pthread N Queens, N = %d, threads = %d\n", n, num_threads);
    printf("Solutions = %lld", count);
    if (n <= KNOWN_MAX) printf(" (%s)", count == KNOWN[n] ? "correct" : "WRONG");
    printf("\nTime taken: %f seconds\n", end - start);

    free(partialCounts);
    return 0;
}
