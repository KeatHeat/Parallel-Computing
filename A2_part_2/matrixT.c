#include "matrixT.h"

int main(int argc, char *argv[])
{
    // declare thread id and thread data
    if (argc != 2) {
        printf("Usage: %s <number of threads>\n", argv[0]);
        return 1;
    }

    num_threads = atoi(argv[1]);

    if (num_threads < 1 || num_threads > 4) {
        printf("Number of threads must be between 1 and 4.\n");
        return 1;
    }
    
    // Dynamically allocate memory for the matrices
    A = (int**)malloc(N * sizeof(int*));
    B = (int**)malloc(N * sizeof(int*));
    C = (int**)malloc(N * sizeof(int*));

    for (int i = 0; i < N; ++i) {
        A[i] = (int*)malloc(N * sizeof(int));
        B[i] = (int*)malloc(N * sizeof(int));
        C[i] = (int*)malloc(N * sizeof(int));
    }

    if (A == NULL || B == NULL || C == NULL) {
        printf("Memory allocation failed!\n");
        return -1;
    }

    // Initialize matrices A and B with values
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            A[i][j] = 1;
            B[i][j] = 1;
            C[i][j] = 0;
        }
    }

    printf("Matrices initialized successfully.\n");

    pthread_t threads[num_threads];
    thread_data_t thread_data[num_threads];

    // Create threads to perform matrix multiplication
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].num_rows = N / num_threads;

        pthread_create(
            &threads[i],
            NULL,
            matrixMultiplyThread,
            &thread_data[i]
        );
    }

    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }


    printf("Matrix multiplication complete!\n");

    //Verify results
    printf("C[0][0] = %d\n", C[0][0]);
    printf("C[999][999] = %d\n", C[999][999]);

    // Optionally, display the resulting matrix C (Not when you are timing :) )
    //displayMatrix(C, N);

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