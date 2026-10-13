// ONE APPRAOCH OF PARALLELIZING THE ORIGINAL BUBBLE SORT

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE 100000
#define MAX_VAL 100000  // max random number

int array[SIZE];
int original[SIZE];   // copy of unsorted data so sequential + parallel sort the same input
int num_threads = 4;  // set from command line
pthread_barrier_t barrier;

typedef struct {
    int start, end;
} ThreadData;

void printArray() 
{
    for (int i = 0; i < SIZE; i++) 
        printf("%d ", array[i]);
    printf("\n");
}

// Each thread bubble sorts its own segment for SIZE passes
void *parallel_bubble(void *arg) 
{
    ThreadData *data = (ThreadData *)arg;
    int start = data->start;
    int end = data->end;

    for (int i = 0; i < SIZE - 1; i++) {
        for (int j = start; j < end - 1; j++) {
            if (array[j] > array[j + 1]) {
                int tmp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = tmp;
            }
        }

        // Compare across the boundary with the next thread's segment
        if (data->end < SIZE) {
            int j = end - 1;
            if (array[j] > array[j + 1]) {
                int tmp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = tmp;
            }
        }

        // wait for every thread to finish this pass before starting the next
        pthread_barrier_wait(&barrier);
    }

    return NULL;
}

// Sequential bubble sort used as the baseline for benchmarking
void bubble_seq(int *a, int n)
{
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (a[j] > a[j + 1]) {
                int tmp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = tmp;
            }
}

// Returns 1 if the array is in non-decreasing order
int is_sorted(int *a, int n)
{
    for (int i = 1; i < n; i++)
        if (a[i - 1] > a[i]) return 0;
    return 1;
}

double elapsed_ms(struct timespec s, struct timespec e)
{
    return (e.tv_sec - s.tv_sec) * 1000.0 + (e.tv_nsec - s.tv_nsec) / 1e6;
}

int main(int argc, char *argv[])
{
    if (argc > 1) num_threads = atoi(argv[1]);
    if (num_threads < 1 || num_threads > 4) {
        fprintf(stderr, "Thread count must be between 1 and 4.\n");
        return 1;
    }

    for (int i = 0; i < SIZE; i++)
        original[i] = rand() % MAX_VAL;

    struct timespec start, end;

    // ----- sequential baseline -----
    memcpy(array, original, sizeof(array));
    clock_gettime(CLOCK_MONOTONIC, &start);
    bubble_seq(array, SIZE);
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Sequential Bubble Time: %.3f ms (sorted: %s)\n",
           elapsed_ms(start, end), is_sorted(array, SIZE) ? "YES" : "NO");

    // ----- parallel version -----
    memcpy(array, original, sizeof(array));

    pthread_t threads[num_threads];
    ThreadData tdata[num_threads];
    pthread_barrier_init(&barrier, NULL, num_threads);

    clock_gettime(CLOCK_MONOTONIC, &start);
    int chunk_size = SIZE / num_threads;
    for (int i = 0; i < num_threads; i++) {
        tdata[i].start = i * chunk_size;
        tdata[i].end = (i == num_threads - 1) ? SIZE : (i + 1) * chunk_size;
        pthread_create(&threads[i], NULL, parallel_bubble, &tdata[i]);
    }

    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &end);

    pthread_barrier_destroy(&barrier);

    printf("Parallel Bubble_1 Time (%d threads): %.3f ms (sorted: %s)\n",
           num_threads, elapsed_ms(start, end), is_sorted(array, SIZE) ? "YES" : "NO");

    return 0;
}
