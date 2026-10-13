#include "p_merge.h"
#include <stdbool.h>

// Merges the two sorted halves arr[l..m] and arr[m+1..r]
void merge(int* arr, int l, int m, int r) 
{
    int n1 = m - l + 1;
    int n2 = r - m;

    int* L = (int*)malloc(n1 * sizeof(int));
    int* R = (int*)malloc(n2 * sizeof(int));

    for (int i = 0; i < n1; i++) L[i] = arr[l + i];
    for (int i = 0; i < n2; i++) R[i] = arr[m + 1 + i];

    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j])
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    free(L);
    free(R);
}

// Sequential recursive merge sort
void merge_sort_seq(int* arr, int l, int r) {
    if (l < r) {
        int m = (l + r) / 2;
        merge_sort_seq(arr, l, m);
        merge_sort_seq(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

// ===== APPROACH 1: SEGMENT-BASED =====
// Thread function: each thread sorts its own segment sequentially
void* parallel_merge_sort(void* arg)
{
    thread_data_t* data = (thread_data_t*)arg;

    merge_sort_seq(data->arr, data->left, data->right);

    return NULL;
}


// Splits the array into one segment per thread, sorts the segments
// in parallel, then merges the sorted segments together
void merge_sort_p(int* arr, int l, int r, int num_threads)
{
    int length = r - l + 1;

    if (num_threads > length) {
        num_threads = length;
    }

    pthread_t threads[num_threads];
    thread_data_t thread_data[num_threads];

    int segment_size = length / num_threads;

    // Create one thread for each segment.
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].arr = arr;
        thread_data[i].left = l + i * segment_size;
        thread_data[i].right = (i == num_threads - 1)
            ? r
            : l + (i + 1) * segment_size - 1;

        if (pthread_create(&threads[i], NULL, parallel_merge_sort,
                           &thread_data[i]) != 0) {
            fprintf(stderr, "Error creating merge sort thread\n");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all segments to finish sorting.
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Merge the sorted segments, combining larger ranges each pass.
    for (int width = segment_size; width < length; width *= 2) {
        for (int left = l; left <= r - width; left += 2 * width) {
            int mid = left + width - 1;
            int right = (left + 2 * width - 1 < r)
                ? left + 2 * width - 1
                : r;

            merge(arr, left, mid, right);
        }
    }
}

// ===== APPROACH 2: RECURSIVE THREAD CREATION =====
// Shared counter of active threads, protected by recursive_mutex
static pthread_mutex_t recursive_mutex = PTHREAD_MUTEX_INITIALIZER;
static int recursive_active_threads = 0;
static int recursive_max_threads = 1;

// Thread function: sorts the left half, then lowers the thread count
static void* recursive_sort_worker(void* arg)
{
    thread_data_t* data = (thread_data_t*)arg;

    merge_sort_recursive_threads(
        data->arr,
        data->left,
        data->right,
        data->level,
        data->max_depth
    );

    pthread_mutex_lock(&recursive_mutex);
    recursive_active_threads--;
    pthread_mutex_unlock(&recursive_mutex);

    return NULL;
}

// Creates a thread for the left half while this thread sorts the right
// half. Falls back to sequential once the thread limit is reached.
void merge_sort_recursive_threads(
    int* arr, int l, int r, int depth, int max_depth)
{
    if (l >= r) {
        return;
    }

    int m = l + (r - l) / 2;
    pthread_t left_thread;
    int left_created = 0;

    thread_data_t left_data = {arr, l, m, depth + 1, max_depth};

    // Only create threads in the top levels of the recursion (1 - 2 - 4)
    if (depth < max_depth) {
        pthread_mutex_lock(&recursive_mutex);
        if (recursive_active_threads < recursive_max_threads - 1) {
            recursive_active_threads++;

            if (pthread_create(&left_thread, NULL,
                               recursive_sort_worker, &left_data) == 0) {
                left_created = 1;
            } else {
                recursive_active_threads--;
            }
        }
        pthread_mutex_unlock(&recursive_mutex);
    }

    // Sort the right half here, then wait for the left half thread
    if (left_created) {
        merge_sort_recursive_threads(arr, m + 1, r, depth + 1, max_depth);
        pthread_join(left_thread, NULL);
    } else {
        merge_sort_recursive_threads(arr, l, m, depth + 1, max_depth);
        merge_sort_recursive_threads(arr, m + 1, r, depth + 1, max_depth);
    }

    merge(arr, l, m, r);
}

// Sets the thread limit and starts the recursive sort
void merge_sort_recursive_limited(int* arr, int l, int r, int num_threads)
{
    pthread_mutex_lock(&recursive_mutex);
    recursive_max_threads = num_threads;
    recursive_active_threads = 0;
    pthread_mutex_unlock(&recursive_mutex);

    // Threads double each level, so only log2(num_threads) levels create threads
    int max_depth = 0;
    while ((1 << max_depth) < num_threads) {
        max_depth++;
    }

    merge_sort_recursive_threads(arr, l, r, 0, max_depth);
}

void printArray(int* arr, int n) 
{
    for (int i = 0; i < n; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

double time_diff_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_nsec - start.tv_nsec) / 1000000.0;
}




// ===== APPROACH 3: THREAD POOL =====
// A task is either sorting a segment or merging two sorted ranges
typedef struct {
    int *arr;
    int left;
    int mid;
    int right;
    int is_merge;
} sort_task_t;

// Shared pool state: the task list and the sync variables
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t task_available;
    pthread_cond_t tasks_complete;
    sort_task_t *tasks;
    int task_count;
    int next_task;
    int active_tasks;
    bool shutdown;
} thread_pool_t;

static thread_pool_t pool;
static pthread_t *pool_threads;
static int pool_size = 0;
static bool pool_initialized = false;

// Worker loop: wait for a task, run it, report when it is done.
// Workers stay alive and are reused until the pool is shut down.
static void *pool_worker(void *arg)
{
    (void)arg;

    while (1) {
        pthread_mutex_lock(&pool.mutex);

        // Sleep until there is a task or the pool is shutting down
        while (pool.next_task >= pool.task_count && !pool.shutdown) {
            pthread_cond_wait(&pool.task_available, &pool.mutex);
        }

        if (pool.shutdown) {
            pthread_mutex_unlock(&pool.mutex);
            return NULL;
        }

        sort_task_t task = pool.tasks[pool.next_task++];
        pool.active_tasks++;

        pthread_mutex_unlock(&pool.mutex);

        if (task.is_merge) {
            merge(task.arr, task.left, task.mid, task.right);
        } else {
            merge_sort_seq(task.arr, task.left, task.right);
        }

        pthread_mutex_lock(&pool.mutex);
        pool.active_tasks--;

        // Wake up the main thread when the last task is finished
        if (pool.next_task >= pool.task_count &&
            pool.active_tasks == 0) {
            pthread_cond_signal(&pool.tasks_complete);
        }

        pthread_mutex_unlock(&pool.mutex);
    }
}

// Creates the worker threads once at the start
static int create_thread_pool(int num_threads)
{
    pool_threads = malloc(num_threads * sizeof(pthread_t));
    if (pool_threads == NULL) {
        return 0;
    }

    pthread_mutex_init(&pool.mutex, NULL);
    pthread_cond_init(&pool.task_available, NULL);
    pthread_cond_init(&pool.tasks_complete, NULL);

    pool.task_count = 0;
    pool.next_task = 0;
    pool.active_tasks = 0;
    pool.shutdown = false;
    pool_size = 0;

    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&pool_threads[i], NULL, pool_worker, NULL) != 0) {
            break;
        }
        pool_size++;
    }

    if (pool_size == 0) {
        free(pool_threads);
        pool_threads = NULL;
        pthread_cond_destroy(&pool.tasks_complete);
        pthread_cond_destroy(&pool.task_available);
        pthread_mutex_destroy(&pool.mutex);
        return 0;
    }

    pool_initialized = true;
    return 1;
}

// Hands a list of tasks to the pool and waits until all are done
static void run_pool_tasks(sort_task_t *tasks, int task_count)
{
    pthread_mutex_lock(&pool.mutex);

    pool.tasks = tasks;
    pool.task_count = task_count;
    pool.next_task = 0;
    pool.active_tasks = 0;

    pthread_cond_broadcast(&pool.task_available);

    while (pool.next_task < pool.task_count ||
           pool.active_tasks > 0) {
        pthread_cond_wait(&pool.tasks_complete, &pool.mutex);
    }

    pthread_mutex_unlock(&pool.mutex);
}

// Tells the workers to stop, joins them, and frees the pool
void destroy_thread_pool(void)
{
    if (!pool_initialized) {
        return;
    }

    pthread_mutex_lock(&pool.mutex);
    pool.shutdown = true;
    pthread_cond_broadcast(&pool.task_available);
    pthread_mutex_unlock(&pool.mutex);

    for (int i = 0; i < pool_size; i++) {
        pthread_join(pool_threads[i], NULL);
    }

    pthread_cond_destroy(&pool.tasks_complete);
    pthread_cond_destroy(&pool.task_available);
    pthread_mutex_destroy(&pool.mutex);

    free(pool_threads);
    pool_threads = NULL;
    pool_size = 0;
    pool_initialized = false;
}

// Sorts the segments with the pool, then merges them level by level,
// running each level's merges in parallel on the pool
void merge_sort_thread_pool(int *arr, int l, int r, int num_threads)
{
    if (l >= r || num_threads < 1) {
        return;
    }

    if (!pool_initialized && !create_thread_pool(num_threads)) {
        fprintf(stderr, "Unable to create thread pool\n");
        exit(EXIT_FAILURE);
    }

    int length = r - l + 1;
    int segments = pool_size;

    if (segments > length) {
        segments = length;
    }

    int segment_size = length / segments;

    sort_task_t *tasks = malloc(segments * sizeof(sort_task_t));
    if (tasks == NULL) {
        fprintf(stderr, "Unable to allocate sort tasks\n");
        exit(EXIT_FAILURE);
    }

    int actual_segments = 0;

    for (int i = 0; i < segments; i++) {
        int left = l + i * segment_size;

        if (left > r) {
            break;
        }

        int right = (i == segments - 1)
        ? r
        : left + segment_size - 1;

        tasks[actual_segments++] = (sort_task_t){
            arr, left, 0, right, 0
        };
    }

    // Run all of the segment sorts
    run_pool_tasks(tasks, actual_segments);
    free(tasks);

    // Each pass merges pairs of sorted ranges, doubling the width
    for (int width = segment_size; width < length; width *= 2) {
        int merge_count = 0;

        for (int left = l; left <= r - width; left += 2 * width) {
            merge_count++;
        }

        if (merge_count == 0) {
            break;
        }

        tasks = malloc(merge_count * sizeof(sort_task_t));
        if (tasks == NULL) {
            fprintf(stderr, "Unable to allocate merge tasks\n");
            exit(EXIT_FAILURE);
        }

        int index = 0;

        for (int left = l; left <= r - width; left += 2 * width) {
            int mid = left + width - 1;
            int right = left + 2 * width - 1;

            if (right > r) {
                right = r;
            }

            tasks[index++] = (sort_task_t){
                arr, left, mid, right, 1
            };
        }

        run_pool_tasks(tasks, merge_count);
        free(tasks);
    }
}


