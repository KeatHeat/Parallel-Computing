// trapT.c - Trapezoidal rule integration parallelized with pthreads
// Integrates f(x) = 4/(1+x^2) from 0 to 1, which estimates pi (Assignment 1 function).
//
// Compile: gcc -Wall -Wextra -O2 trapT.c -o trapT -pthread
// Run:     ./trapT <threads> [n]   (default n = 1,000,000,000)

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

long long n;          // number of trapezoids
double h;             // width of each trapezoid
int num_threads;
double *partialSums;  // one partial sum per thread

// Each thread sums f at its share of the interior points 1..n-1
void *trapPart(void *arg)
{
    int id = *(int *)arg;

    long long interior = n - 1;
    long long chunk = interior / num_threads;
    long long first = 1 + id * chunk;
    long long last = (id == num_threads - 1) ? n - 1 : first + chunk - 1;

    double local = 0.0;
    for (long long i = first; i <= last; i++) {
        local += f(A_START + i * h);
    }
    partialSums[id] = local;
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <threads> [n]\n", argv[0]);
        return 1;
    }

    num_threads = atoi(argv[1]);
    n = (argc > 2) ? atoll(argv[2]) : DEFAULT_N;
    if (num_threads < 1 || n <= 0) {
        printf("Threads and n must be positive.\n");
        return 1;
    }

    h = (B_END - A_START) / n;
    partialSums = calloc(num_threads, sizeof(double));
    pthread_t threads[num_threads];
    int ids[num_threads];

    double start = now_seconds();

    // Create threads, each summing its chunk of interior points
    for (int i = 0; i < num_threads; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, trapPart, &ids[i]);
    }

    // Wait for all threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Combine: endpoints count half, then add every thread's partial sum
    double sum = (f(A_START) + f(B_END)) / 2.0;
    for (int i = 0; i < num_threads; i++) {
        sum += partialSums[i];
    }
    double result = sum * h;

    double end = now_seconds();

    printf("Pthread trapezoidal rule, n = %lld, threads = %d\n", n, num_threads);
    printf("Estimated pi (integral from %.1f to %.1f) = %.12f (actual %.12f)\n",
           A_START, B_END, result, EXACT);
    printf("Time taken: %f seconds\n", end - start);

    free(partialSums);
    return 0;
}
