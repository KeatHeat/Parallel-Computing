// mainM.c - Sequential matrix multiplication from Assignment 1 (baseline for Task 2)
//
// Compile: gcc -Wall -Wextra -O2 mainM.c -o mainM
// Run:     ./mainM
//
// Change from Assignment 1: timing with clock_gettime(CLOCK_MONOTONIC)
// around matrixMultiply, and a check of the result.

#include "matrixMul.h"
#include <time.h>

// Current time in seconds, using CLOCK_MONOTONIC as the assignment asks
static double now_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(void)
{
    // Dynamically allocate memory for the matrices
    int **A = (int **)malloc(N * sizeof(int *));
    int **B = (int **)malloc(N * sizeof(int *));
    int **C = (int **)malloc(N * sizeof(int *));
    if (A == NULL || B == NULL || C == NULL) {
        printf("Memory allocation failed!\n");
        return -1;
    }
    for (int i = 0; i < N; ++i) {
        A[i] = (int *)malloc(N * sizeof(int));
        B[i] = (int *)malloc(N * sizeof(int));
        C[i] = (int *)malloc(N * sizeof(int));
    }

    // Initialize matrices A and B
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            A[i][j] = 1;
            B[i][j] = 1;
            C[i][j] = 0;
        }
    }

    double start = now_seconds();
    matrixMultiply(A, B, C, N);
    double end = now_seconds();

    printf("Sequential matrix multiplication, N = %d\n", N);
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
