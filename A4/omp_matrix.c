// omp_matrix.c - Matrix multiplication parallelized with OpenMP (Assignment 3, Task 2)
//
// Compile: gcc -Wall -Wextra -O2 -fopenmp omp_matrix.c -o omp_matrix
// Run:     ./omp_matrix <threads>
// Scheduling experiment (Task 6):  OMP_SCHEDULE=guided ./omp_matrix 4

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000  // Size of the matrix

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <threads>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads < 1) {
        printf("Number of threads must be at least 1.\n");
        return 1;
    }

    // Dynamically allocate memory for the matrices
    int **A = malloc(N * sizeof(int *));
    int **B = malloc(N * sizeof(int *));
    int **C = malloc(N * sizeof(int *));
    if (!A || !B || !C) {
        printf("Memory allocation failed!\n");
        return 1;
    }
    for (int i = 0; i < N; ++i) {
        A[i] = malloc(N * sizeof(int));
        B[i] = malloc(N * sizeof(int));
        C[i] = malloc(N * sizeof(int));
    }

    // Initialize matrices A and B with 1s
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            A[i][j] = 1;
            B[i][j] = 1;
        }
    }

    omp_set_num_threads(num_threads);

    // schedule(runtime) below reads OMP_SCHEDULE; default to static if unset
    if (!getenv("OMP_SCHEDULE")) {
        omp_set_schedule(omp_sched_static, 0);
    }

    // Start timing
    double start_time = omp_get_wtime();

    // Split the rows (i loop) across threads. j, k and sum are declared
    // inside the loop, so each thread automatically gets its own copies.
    // Each thread writes different rows of C, so no locking is needed.
    #pragma omp parallel for schedule(runtime)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < N; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }

    // End timing
    double end_time = omp_get_wtime();

    printf("OpenMP matrix multiplication, N = %d, threads = %d\n", N, num_threads);
    printf("C[0][0] = %d, C[%d][%d] = %d (expected %d)\n",
           C[0][0], N - 1, N - 1, C[N - 1][N - 1], N);
    printf("Time taken: %f seconds\n", end_time - start_time);

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
