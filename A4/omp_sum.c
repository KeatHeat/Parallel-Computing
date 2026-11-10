// omp_sum.c - Array sum parallelized with OpenMP (Assignment 3, Task 1)
//
// Compile: gcc -Wall -Wextra -O2 -fopenmp omp_sum.c -o omp_sum
// Run:     ./omp_sum <threads> [N]   (default N = 1,000,000,000)
// Scheduling experiment (Task 6):  OMP_SCHEDULE=dynamic,10000 ./omp_sum 4

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <threads> [N]\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    long long N = (argc > 2) ? atoll(argv[2]) : 1000000000LL;

    if (N <= 0 || num_threads < 1) {
        fprintf(stderr, "Threads and N must be positive.\n");
        return 1;
    }

    // 1,000,000,000 ints is about 4 GB
    int *arr = malloc((size_t)N * sizeof(int));
    if (!arr) {
        perror("malloc");
        return 1;
    }

    // Fill the array with 1..N (not timed)
    for (long long i = 0; i < N; i++) {
        arr[i] = (int)(i + 1);
    }

    // Tell OpenMP how many threads to use
    omp_set_num_threads(num_threads);

    // schedule(runtime) below reads OMP_SCHEDULE; default to static if unset
    if (!getenv("OMP_SCHEDULE")) {
        omp_set_schedule(omp_sched_static, 0);
    }

    long long total = 0;

    // Start timing
    double start_time = omp_get_wtime();

    // Each thread sums part of the loop into its own private copy of 'total';
    // reduction(+:total) adds the private copies together at the end.
    #pragma omp parallel for reduction(+:total) schedule(runtime)
    for (long long i = 0; i < N; i++) {
        total += arr[i];
    }

    // End timing
    double end_time = omp_get_wtime();

    long long expected = N * (N + 1) / 2;
    printf("OpenMP array sum, N = %lld, threads = %d\n", N, num_threads);
    printf("Total sum = %lld (%s)\n", total, total == expected ? "correct" : "WRONG");
    printf("Time taken: %f seconds\n", end_time - start_time);

    free(arr);
    return 0;
}
