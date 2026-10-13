#ifndef PARALLEL_SORTS_H
#define PARALLEL_SORTS_H

#define SIZE 100000
#define MAX_VAL 100000  // max random number

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

// Data passed to each thread: the array, the range to sort,
// and the current/max recursion depth (used by approach 2)
typedef struct {
    int* arr;
    int left;
    int right;
    int level;
    int max_depth;
} thread_data_t;

// Sequential merge sort
void merge(int* arr, int l, int m, int r);
void merge_sort_seq(int* arr, int l, int r);

// Approach 1: segment-based
void merge_sort_p(int* arr, int l, int r, int num_threads);
void* parallel_merge_sort(void* arg);

// Approach 2: recursive thread creation
void merge_sort_recursive_threads(int* arr, int l, int r, int depth, int max_depth);
// Approach 3: thread pool
void merge_sort_thread_pool(int* arr, int l, int r, int num_threads);
void destroy_thread_pool(void);
void merge_sort_recursive_limited(int* arr, int l, int r, int num_threads);

void printArray(int* arr, int n);
double time_diff_ms(struct timespec start, struct timespec end);

#endif