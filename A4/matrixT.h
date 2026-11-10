// matrixT.h - Header for the pthread matrix multiplication
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 1000  // Size of the matrix

int **A, **B, **C;  // Global matrices
int num_threads;    // Number of threads to be used

// Structure to hold information for each thread
typedef struct
{
    int thread_id;
} thread_data_t;

// Function for each thread to perform matrix multiplication
void *matrixMultiplyThread(void *arg)
{
    // Divide the rows among the threads based on thread id.
    // Integer math spreads leftover rows when N doesn't divide evenly.
    thread_data_t *data = (thread_data_t *)arg;

    int start_row = data->thread_id * N / num_threads;
    int end_row = (data->thread_id + 1) * N / num_threads;

    // Compute this thread's rows of C
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < N; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }

    return NULL;
}

void displayMatrix(int **matrix, int n)
{
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}
