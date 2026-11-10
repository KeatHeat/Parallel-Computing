// seq_sum.c - Sequential array sum (Assignment 1 arraysum.c, baseline for Task 1)
//
// Changes from Assignment 1: N = 1,000,000,000 needs a malloc'd array
// (a stack array that big would crash) and a long long sum (int overflows).
//
// Compile: gcc -Wall -Wextra -O2 seq_sum.c -o seq_sum
// Run:     ./seq_sum [N]        (default N = 1,000,000,000)

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

// Same function as Assignment 1, with long long types for the bigger N
long long sumArray(int arr[], long long size)
{
    long long sum = 0;

    for (long long i = 0; i < size; i++) {
        sum += arr[i];
    }

    return sum;
}

int main(int argc, char *argv[])
{
    long long N = (argc > 1) ? atoll(argv[1]) : 1000000000LL;
    if (N <= 0) {
        fprintf(stderr, "N must be positive.\n");
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

    // Time only the summing loop
    double start = now_seconds();

    long long total = sumArray(arr, N);

    double end = now_seconds();

    long long expected = N * (N + 1) / 2;
    printf("Sequential array sum, N = %lld\n", N);
    printf("Total sum = %lld (%s)\n", total, total == expected ? "correct" : "WRONG");
    printf("Time taken: %f seconds\n", end - start);

    free(arr);
    return 0;
}
