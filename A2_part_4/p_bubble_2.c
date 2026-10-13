// ANOTHER APPROACH OF PARALLELIZING BUBBLE SORT (USES A VARIATION OF BUBBLE SORT - ODD/EVEN / BRICK SORT)

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define SIZE 100000
#define MAX_VAL 100000  // max random number

int *array;
int array_size;
int num_threads = 4;  // set from command line
pthread_barrier_t barrier;

void printArray(int* arr, int n) 
{
    for (int i = 0; i < n; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

// Odd-even transposition sort: alternates even and odd phases
void *parallel_odd_even(void *arg) 
{
    long thread_id = *(long *)arg;
    free(arg);

    int chunk_size = array_size / num_threads;
    int start_index = thread_id * chunk_size;
    int end_index = (thread_id == num_threads - 1) ? array_size : start_index + chunk_size;

    // Even phases compare (0,1),(2,3)... odd phases compare (1,2),(3,4)...
    for (int phase = 0; phase < array_size; ++phase) {
        int is_even_phase = (phase % 2 == 0);

        for (int j = start_index + is_even_phase; j < end_index - 1; j += 2) {
            if (array[j] > array[j + 1]) {
                int temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }

        // Handle the pair that crosses into the next thread's section
        if (thread_id != num_threads - 1) {
            int j = end_index - 1;
            if ((j % 2 == is_even_phase) && array[j] > array[j + 1]) {
                int temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }

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

    srand(time(NULL));
    array_size = SIZE;
    array = malloc(array_size * sizeof(int));
    int *original = malloc(array_size * sizeof(int));
    if (!array || !original) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < SIZE; i++)
        original[i] = rand() % MAX_VAL;
    //printf("Unsorted array: "); printArray(original, SIZE);

    struct timespec start, end;

    // ----- sequential baseline -----
    memcpy(array, original, SIZE * sizeof(int));
    clock_gettime(CLOCK_MONOTONIC, &start);
    bubble_seq(array, SIZE);
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Sequential Bubble Time: %.3f ms (sorted: %s)\n",
           elapsed_ms(start, end), is_sorted(array, SIZE) ? "YES" : "NO");

    // ----- parallel odd-even version -----
    memcpy(array, original, SIZE * sizeof(int));

    if (pthread_barrier_init(&barrier, NULL, num_threads)) {
        perror("Barrier init failed");
        free(array);
        exit(EXIT_FAILURE);
    }

    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_t threads[num_threads];
    for (long i = 0; i < num_threads; i++) {
        long *tid = malloc(sizeof(long));
        if (!tid)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        *tid = i;
        if (pthread_create(&threads[i], NULL, parallel_odd_even, tid) != 0) {
            perror("pthread_create failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &end);

    pthread_barrier_destroy(&barrier);

    printf("Parallel Bubble_2 Time (%d threads): %.3f ms (sorted: %s)\n",
           num_threads, elapsed_ms(start, end), is_sorted(array, SIZE) ? "YES" : "NO");
    //printf("Sorted array:   "); printArray(array, SIZE);

    free(original);
    free(array);
    return 0;
}
