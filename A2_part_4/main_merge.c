#include "p_merge.h"

int main(int argc, char* argv[])
{
    int num_threads = 4;

    // Read the number of threads from the command line (default 4)
    if (argc > 1) {
        num_threads = atoi(argv[1]);
    }

    if (num_threads < 1 || num_threads > 4) {
        fprintf(stderr, "Thread count must be between 1 and 4.\n");
        return EXIT_FAILURE;
    }

    struct timespec start, end;

    // One array per sort so every approach sorts the same input
    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    int* arr3 = (int*)malloc(SIZE * sizeof(int));
    int* arr4 = (int*)malloc(SIZE * sizeof(int));

    for (int i = 0; i < SIZE; i++) 
    {
        int val = rand() % MAX_VAL;
        arr1[i] = val;
        arr2[i] = val;
        arr3[i] = val;
        arr4[i] = val;
    }

    // Sequential merge sort (baseline)
    clock_gettime(CLOCK_MONOTONIC, &start);
    merge_sort_seq(arr1, 0, SIZE - 1);
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Sequential Merge Sort Time: %.3f ms\n", time_diff_ms(start, end));
    //printArray(arr1, SIZE);


    // Approach 1: segment-based
    clock_gettime(CLOCK_MONOTONIC, &start);
    merge_sort_p(arr2, 0, SIZE - 1, num_threads);
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Parallel Merge Sort Time: %.3f ms\n", time_diff_ms(start, end));
    //printArray(arr2, SIZE);

    // Approach 2: recursive thread creation
    clock_gettime(CLOCK_MONOTONIC, &start);
    merge_sort_recursive_limited(arr3, 0, SIZE - 1, num_threads);
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Recursive Merge Sort Time: %.3f ms\n", time_diff_ms(start, end));

    // Approach 3: thread pool
    clock_gettime(CLOCK_MONOTONIC, &start);
    merge_sort_thread_pool(arr4, 0, SIZE - 1, num_threads);
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Thread Pool Merge Sort Time: %.3f ms\n", time_diff_ms(start, end));

    // Check that all four sorted arrays are identical
    int correct = 1;

    for (int i = 0; i < SIZE; i++) {
        if (arr1[i] != arr2[i] ||
            arr1[i] != arr3[i] ||
            arr1[i] != arr4[i]) {
            correct = 0;
            break;
        }
    }

    printf("All sorting approaches match: %s\n",correct ? "YES" : "NO");

    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);

    // Shut down and join the pool threads created in approach 3
    destroy_thread_pool();

    return 0;
}
