// numIntegrate.c - Sequential trapezoidal rule from Assignment 1 (baseline for Task 3)
// Integrates f(x) = 4/(1+x^2) from 0 to 1, which estimates pi (Assignment 1 function).
//
// Compile: gcc -Wall -Wextra -O2 numIntegrate.c -o numIntegrate
// Run:     ./numIntegrate [n]     (default n = 1,000,000,000)

#include <stdio.h>
#include <stdlib.h>
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

int main(int argc, char *argv[])
{
    long long n = (argc > 1) ? atoll(argv[1]) : DEFAULT_N;
    if (n <= 0) {
        fprintf(stderr, "n must be positive.\n");
        return 1;
    }

    double a = A_START, b = B_END;
    double h = (b - a) / n;   // width of each trapezoid

    double start = now_seconds();

    // Endpoints count half, interior points count fully
    double sum = (f(a) + f(b)) / 2.0;
    for (long long i = 1; i < n; i++) {
        sum += f(a + i * h);
    }
    double result = sum * h;

    double end = now_seconds();

    printf("Sequential trapezoidal rule, n = %lld\n", n);
    printf("Estimated pi (integral from %.1f to %.1f) = %.12f (actual %.12f)\n", a, b, result, EXACT);
    printf("Time taken: %f seconds\n", end - start);
    return 0;
}
