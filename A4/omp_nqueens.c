// omp_nqueens.c - N Queens parallelized with OpenMP (Assignment 3, Task 4)
//
// Compile: gcc -Wall -Wextra -O2 -fopenmp omp_nqueens.c -o omp_nqueens
// Run:     ./omp_nqueens <threads> [N]   (default N = 15)
// Scheduling experiment (Task 6):  OMP_SCHEDULE=dynamic ./omp_nqueens 4
//
// The search is split into n*n tasks (first two queens fixed). Tasks take
// very different amounts of time, which makes this the best program for
// comparing static, dynamic and guided scheduling.

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "nqueens.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <threads> [N]\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    int n = (argc > 2) ? atoi(argv[2]) : 15;
    if (num_threads < 1 || n < 2 || n > MAXN) {
        printf("Threads must be >= 1 and N between 2 and %d.\n", MAXN);
        return 1;
    }

    omp_set_num_threads(num_threads);

    // schedule(runtime) below reads OMP_SCHEDULE; default to static if unset
    if (!getenv("OMP_SCHEDULE")) {
        omp_set_schedule(omp_sched_static, 0);
    }

    long long count = 0;

    // Start timing
    double start_time = omp_get_wtime();

    // Each task builds its own board inside solve_task, so threads share
    // nothing; reduction(+:count) adds up every thread's solution count.
    #pragma omp parallel for reduction(+:count) schedule(runtime)
    for (int t = 0; t < n * n; t++) {
        count += solve_task(n, t);
    }

    // End timing
    double end_time = omp_get_wtime();

    printf("OpenMP N Queens, N = %d, threads = %d\n", n, num_threads);
    printf("Solutions = %lld", count);
    if (n <= KNOWN_MAX) printf(" (%s)", count == KNOWN[n] ? "correct" : "WRONG");
    printf("\nTime taken: %f seconds\n", end_time - start_time);
    return 0;
}
