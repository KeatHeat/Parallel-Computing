#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 1000  // Size of the matrix
#define NUM_THREADS 4  // Number of threads

int **A, **B, **C;  // Global matrices
int num_threads;  // Number of threads to be used

// Structure to hold information for each thread
typedef struct 
{
    int thread_id;
    int num_rows;  // Number of rows each thread will handle
} thread_data_t;

// Function for each thread to perform matrix multiplication
void* matrixMultiplyThread(void* arg) 
{
    // Divide the task (rows) of each thread based on thread id
    thread_data_t* data = (thread_data_t*)arg;

    int start_row = data->thread_id * N / num_threads;
    int end_row = (data->thread_id + 1) * N / num_threads;

    // compute a portion of the matrix multiplication
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return NULL;
}

void displayMatrix(int** matrix, int n) 
{
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}