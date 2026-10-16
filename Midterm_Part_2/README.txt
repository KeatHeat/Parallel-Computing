Midterm - Part 2
Parallel Computing

Name: Keaton Hagen
Course: CSC 410 Parallel Computing
Semester: Fall 2026

Task 1 - sobel_pthread.c and sobel_openmp.c
-------------------------------------------
Both programs run the Sobel edge detection operator on a color image.
Each program first runs the original sequential version for comparison,
then runs the parallel version on the same image. Both outputs are
compared byte by byte and the program prints whether they match. The
parallel result is saved as a .bmp file.

Every output pixel only reads from the input image and only writes to
its own spot in the output image, so no two threads ever write to the
same location. Because of this, neither version needs a mutex.

pthreads (sobel_pthread.c):
The interior rows of the image (row 1 to height - 2) are split into one
block of rows per thread. Rows were used instead of single pixels
because a block of rows is stored next to each other in memory, which
is good for the cache, and it keeps the thread data simple (a start row
and an end row). If the number of rows does not divide evenly, the first
(rows % threads) threads each get one extra row, so no thread does more
than one extra row of work. Each thread gets its own ThreadData struct
with its row range, then main joins all of the threads. Timing uses
clock_gettime(CLOCK_MONOTONIC) since it measures wall clock time and is
not affected by system clock changes.

OpenMP (sobel_openmp.c):
The row and column loops use "#pragma omp parallel for collapse(2)
schedule(static)". collapse(2) combines the two nested loops into one
loop of (height - 2) * (width - 2) pixels, so OpenMP has a lot more
iterations to split between the threads than if only the row loop was
parallelized. schedule(static) gives each thread an equal, continuous
chunk of pixels, which works well because every pixel takes the same
amount of work. The per-pixel work was moved into sobel_pixel() so the
two loops are perfectly nested, which collapse requires. The thread
count is set with omp_set_num_threads() and timing uses omp_get_wtime(),
OpenMP's wall clock timer.

Compile:
gcc -Wall -pthread sobel_pthread.c -lm -o sobel_pthread
gcc -Wall -fopenmp sobel_openmp.c -lm -o sobel_openmp

Run:
./sobel_pthread NGGYU.jpg <num_of_threads>   OR   ./sobel_pthread NGGYU2.jpg <num_of_threads>
./sobel_openmp NGGYU.jpg <num_of_threads>    OR   ./sobel_openmp NGGYU2.jpg <num_of_threads>


Task 2 - rw.c
-------------
This is a custom reader-writer lock built from one mutex and two
condition variables (readers_ok and writers_ok). Many readers can hold
the lock at the same time, but a writer has the lock to itself.

To avoid writer starvation, a new reader has to wait if any writer is
already waiting, even if other readers are currently reading. This lets
the current readers finish so the writer can get in.

To avoid reader starvation, readers and writers take turns. When a
writer finishes, if any readers are waiting, all of them are let in as
one batch (broadcast on readers_ok) before the next writer can go. This
is tracked with two counters:
 - read_turn goes up by 1 every time a writer finishes. A reader saves
   the value when it arrives, so if read_turn has changed, it knows a
   writer has finished since it started waiting and it is now its turn.
 - readers_to_admit is the number of readers in that batch. Writers wait
   until it reaches 0, so a writer cannot cut in front of the batch.
If no readers are waiting when a writer finishes, the next writer is
signaled instead. The last reader to leave signals a waiting writer.

Compile:
gcc -Wall -pthread rw.c -o rw

Run:
./rw


Task 3 - Benchmarks
-------------------
Each version was run 5 times for each image and thread count, and the
average time was recorded. The results and observations are in the
Task 3 pdf.
