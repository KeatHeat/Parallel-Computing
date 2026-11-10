// omp_trap.c - Trapezoidal rule integration parallelized with OpenMP (Assignment 3, Task 3)
// Integrates f(x) = 4/(1+x^2) from 0 to 1, which estimates pi (Assignment 1 function).
//
// Compile: gcc -Wall -Wextra -O2 -fopenmp omp_trap.c -o omp_trap
// Run:     ./omp_trap <threads> [n]   (default n = 1,000,000,000)
// Scheduling experiment (Task 6):  OMP_SCHEDULE=dynamic,10000 ./omp_trap 4

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Same as Assignment 1: integrating 4/(1+x^2) from 0 to 1 gives pi
#define A_START 0.0              // left end of the interval
#define B_END   1.0              // right end of the interval
#define DEFAULT_N 1000000000LL   // number of intervals
#define EXACT 3.14159265358979323846

// Function to integrate
static inline double f(double x)
{
    return 4.0 / (1.0 + x * x);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <threads> [n]\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    long long n = (argc > 2) ? atoll(argv[2]) : DEFAULT_N;
    if (num_threads < 1 || n <= 0) {
        printf("Threads and n must be positive.\n");
        return 1;
    }

    double a = A_START, b = B_END;
    double h = (b - a) / n;   // width of each trapezoid

    omp_set_num_threads(num_threads);

    // schedule(runtime) below reads OMP_SCHEDULE; default to static if unset
    if (!getenv("OMP_SCHEDULE")) {
        omp_set_schedule(omp_sched_static, 0);
    }

    // Start timing
    double start_time = omp_get_wtime();

    // Interior points are split across threads; reduction(+:sum) gives each
    // thread a private sum and adds them together when the loop ends.
    double sum = 0.0;
    #pragma omp parallel for reduction(+:sum) schedule(runtime)
    for (long long i = 1; i < n; i++) {
        sum += f(a + i * h);
    }

    // Endpoints count half (done once, outside the parallel loop)
    sum += (f(a) + f(b)) / 2.0;
    double result = sum * h;

    // End timing
    double end_time = omp_get_wtime();

    printf("OpenMP trapezoidal rule, n = %lld, threads = %d\n", n, num_threads);
    printf("Estimated pi (integral from %.1f to %.1f) = %.12f (actual %.12f)\n", a, b, result, EXACT);
    printf("Time taken: %f seconds\n", end_time - start_time);
    return 0;
}
