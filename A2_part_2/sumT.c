#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define SIZE 100000000

long long arr[SIZE];
long long *partialSums; // Array to store partial sums for each thread
int num_threads;

// Entry function for each thread
void* sumPart(void* arg) 
{
    // Divide the work for each thread based on their id and let them compute partial sums
    int thread_id = *(int*)arg;

    int start = thread_id * (SIZE / num_threads);
    int end = start + (SIZE / num_threads);

    for (int i = start; i < end; i++) {
         partialSums[thread_id] += arr[i];
    }

    return NULL;
}

int main(int argc, char* argv[]) 
{
    if (argc != 2) {
        printf("Usage: %s <number of threads>\n", argv[0]);
        return 1;
    }

    num_threads = atoi(argv[1]);

    if (num_threads < 1 || num_threads > 4) {
        printf("Number of threads must be between 1 and 4.\n");
        return 1;
    }

    partialSums = calloc(num_threads, sizeof(long long));
    
    // Initialize the array
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i + 1; 
    }

    pthread_t threads[num_threads];
    int thread_ids[num_threads];

    // Create threads to compute partial sums
    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;

        pthread_create(&threads[i], NULL, sumPart, &thread_ids[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Combine the partial sums from all threads
    long long totalSum = 0;
    for (int i = 0; i < num_threads; i++) {
        totalSum += partialSums[i];
    }

    // Print the total sum;
    printf("Total Sum: %lld\n", totalSum);

    // Free dynamically allocated memory
    free(partialSums);

    return 0;
}
