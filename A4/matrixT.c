// matrixT.c - Matrix multiplication parallelized with pthreads
//
// Compile: gcc -Wall -Wextra -O2 matrixT.c -o matrixT -pthread
// Run:     ./matrixT <threads>
//
// Changes from Assignment 2: any thread count, and timing with
// clock_gettime(CLOCK_MONOTONIC) around the multiplication only.

#include "matrixT.h"
#include <time.h>

// Current time in seconds, using CLOCK_MONOTONIC as the assignment asks
static double now_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <number of threads>\n", argv[0]);
        return 1;
    }

    num_threads = atoi(argv[1]);
    if (num_threads < 1) {
        printf("Number of threads must be at least 1.\n");
        return 1;
    }

    // Dynamically allocate memory for the matrices
    A = malloc(N * sizeof(int *));
    B = malloc(N * sizeof(int *));
    C = malloc(N * sizeof(int *));
    if (!A || !B || !C) {
        printf("Memory allocation failed!\n");
        return 1;
    }
    for (int i = 0; i < N; ++i) {
        A[i] = malloc(N * sizeof(int));
        B[i] = malloc(N * sizeof(int));
        C[i] = malloc(N * sizeof(int));
    }

    // Initialize matrices A and B with 1s, C with 0s
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            A[i][j] = 1;
            B[i][j] = 1;
            C[i][j] = 0;
        }
    }

    pthread_t threads[num_threads];
    thread_data_t thread_data[num_threads];

    double start = now_seconds();

    // Create threads to perform matrix multiplication
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        pthread_create(&threads[i], NULL, matrixMultiplyThread, &thread_data[i]);
    }

    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    double end = now_seconds();

    // Verify results: every entry should equal N
    printf("Pthread matrix multiplication, N = %d, threads = %d\n", N, num_threads);
    printf("C[0][0] = %d, C[%d][%d] = %d (expected %d)\n",
           C[0][0], N - 1, N - 1, C[N - 1][N - 1], N);
    printf("Time taken: %f seconds\n", end - start);

    // Free dynamically allocated memory
    for (int i = 0; i < N; ++i) {
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }
    free(A);
    free(B);
    free(C);
    return 0;
}
