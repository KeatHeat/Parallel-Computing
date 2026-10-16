// Implement your own read write lock using mutex and condition variables!

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t readers_ok;
    pthread_cond_t writers_ok;
    int readers;          // active readers
    int writers;          // active writers (0 or 1)
    int waiting_writers;  // waiting writers
    int waiting_readers;  // waiting readers
    int read_turn;        // goes up by 1 every time a writer finishes
    int readers_to_admit; // readers let in by the last writer that have not entered yet
} rwlock_t;

rwlock_t lock;
int shared_data = 0;

// --- Custom Reader-Writer Lock Functions ---
// Fairness rules:
//  - New readers wait if a writer is waiting, so a steady stream of
//    readers cannot starve the writers.
//  - When a writer finishes, every reader that was already waiting gets
//    to go next (as one batch) before another writer can get in, so a
//    steady stream of writers cannot starve the readers.

void rwlock_init(rwlock_t *lock) 
{
    pthread_mutex_init(&lock->mutex, NULL);
    pthread_cond_init(&lock->readers_ok, NULL);
    pthread_cond_init(&lock->writers_ok, NULL);

    lock->readers = 0;
    lock->writers = 0;
    lock->waiting_writers = 0;
    lock->waiting_readers = 0;
    lock->read_turn = 0;
    lock->readers_to_admit = 0;
}

void rwlock_acquire_read(rwlock_t *lock) 
{
    pthread_mutex_lock(&lock->mutex);

    // Remember which turn it was when this reader showed up
    int my_turn = lock->read_turn;
    lock->waiting_readers++;

    // Wait while a writer is active, or while a writer is waiting and no
    // writer has finished since this reader arrived (it is not our turn yet)
    while (lock->writers > 0 ||
           (lock->waiting_writers > 0 && lock->read_turn == my_turn)) {
        pthread_cond_wait(&lock->readers_ok, &lock->mutex);
    }

    lock->waiting_readers--;
    lock->readers++;

    // This reader was part of the batch let in by the last writer
    if (lock->read_turn != my_turn && lock->readers_to_admit > 0) {
        lock->readers_to_admit--;
    }

    pthread_mutex_unlock(&lock->mutex);
}

void rwlock_release_read(rwlock_t *lock) 
{
    pthread_mutex_lock(&lock->mutex);

    lock->readers--;

    // Last reader out wakes up one waiting writer
    if (lock->readers == 0 && lock->waiting_writers > 0) {
        pthread_cond_signal(&lock->writers_ok);
    }

    pthread_mutex_unlock(&lock->mutex);
}

void rwlock_acquire_write(rwlock_t *lock) 
{
    pthread_mutex_lock(&lock->mutex);

    lock->waiting_writers++;

    // Wait until nobody is reading or writing, and every reader the last
    // writer let in has had its chance (stops writers from cutting in line)
    while (lock->writers > 0 || lock->readers > 0 || lock->readers_to_admit > 0) {
        pthread_cond_wait(&lock->writers_ok, &lock->mutex);
    }

    lock->waiting_writers--;
    lock->writers = 1;

    pthread_mutex_unlock(&lock->mutex);
}

void rwlock_release_write(rwlock_t *lock) 
{
    pthread_mutex_lock(&lock->mutex);

    lock->writers = 0;

    if (lock->waiting_readers > 0) {
        // Readers that were waiting go next, as one batch
        lock->read_turn++;
        lock->readers_to_admit = lock->waiting_readers;
        pthread_cond_broadcast(&lock->readers_ok);
    } else if (lock->waiting_writers > 0) {
        // No readers waiting, so hand off to the next writer
        pthread_cond_signal(&lock->writers_ok);
    }

    pthread_mutex_unlock(&lock->mutex);
}

void rwlock_destroy(rwlock_t *lock)
{
    pthread_mutex_destroy(&lock->mutex);
    pthread_cond_destroy(&lock->readers_ok);
    pthread_cond_destroy(&lock->writers_ok);
}

// ################################################################################################
// ########## Provided to test your read write lock implementation ################################

void* reader(void* arg) 
{
    int id = *(int*)arg;
    for (int i = 0; i < 5; i++) 
    {
        rwlock_acquire_read(&lock);
        printf("Reader %d reads: %d\n", id, shared_data);
        usleep(100000);
        rwlock_release_read(&lock);
        usleep(100000);
    }
    return NULL;
}

void* writer(void* arg) 
{
    int id = *(int*)arg;
    for (int i = 0; i < 5; i++) 
    {
        rwlock_acquire_write(&lock);
        shared_data += 10;
        printf("Writer %d updates value to: %d\n", id, shared_data);
        usleep(150000); 
        rwlock_release_write(&lock);
        usleep(100000); 
    }
    return NULL;
}

int main() 
{
    pthread_t readers[5], writers[2];
    int r_ids[5], w_ids[2];

    rwlock_init(&lock);

    // reader threads
    for (int i = 0; i < 5; i++) 
    {
        r_ids[i] = i + 1;
        pthread_create(&readers[i], NULL, reader, &r_ids[i]);
    }

    // writer threads
    for (int i = 0; i < 2; i++) 
    {
        w_ids[i] = i + 1;
        pthread_create(&writers[i], NULL, writer, &w_ids[i]);
    }

    for (int i = 0; i < 5; i++)
        pthread_join(readers[i], NULL);
    for (int i = 0; i < 2; i++)
        pthread_join(writers[i], NULL);

    rwlock_destroy(&lock);

    return 0;
}
