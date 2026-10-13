Assignment 2 - Part 4
Still About Synchronization of Threads

Name: Keaton Hagen
Course: CSC 410 Parallel Computing
Semester: Fall 2026

Task 1 - p_integrate.c
----------------------
This program uses the trapezoidal rule to estimate pi by integrating
4 / (1 + x^2) from 0 to 1 using N = 1,000,000,000 intervals.

The intervals are split evenly between the threads. Each thread
calculates its section using a local sum, then adds its local sum to
the shared total once it is finished. A mutex protects the shared total
so that only one thread can update it at a time.

The program first runs a sequential version for comparison, then runs
the parallel version. Both versions produce 3.1415926536.

Compile:
gcc -Wall -pthread p_integrate.c -o p_integrate

Run:
./p_integrate <number_of_threads>     (1 - 4)


Task 2 - p_merge.c, p_merge.h, main_merge.c
-------------------------------------------
This program sorts an array of 100,000 random numbers using a sequential
merge sort and three parallel approaches. All four sorts use the same
input, and the program checks that all of the results match.

Approach 1 (Segment-based) splits the array into N equal segments, where
N is the number of threads. Each thread sorts one segment. After all of
the threads are joined, the sorted segments are merged together.

Approach 2 (Recursive) starts with one thread at the top level. At each
level of recursion a new thread is created for the left half while the
current thread sorts the right half, so the number of threads doubles
(1 - 2 - 4). Threads are only created in the top log2(N) levels of the
recursion, and a mutex protects a counter of active threads. This limits
the total number of threads to N and prevents thread explosion. Below
those levels, the remaining work is done sequentially without locking.

Approach 3 (Thread pool) creates a fixed number of worker threads once
and reuses them. The sorting tasks are added first, then each level of
merge tasks. The workers wait on a condition variable until a task is
available, and the main thread waits on a second condition variable
until all of the tasks are finished. The pool is destroyed at the end
of the program.

Compile:
gcc -Wall -pthread main_merge.c p_merge.c -o main_merge

Run:
./main_merge <number_of_threads>      (1 - 4)


Task 3 - p_bubble_1.c and p_bubble_2.c
--------------------------------------
These are the two given parallel bubble sort versions. Timing, a
sequential bubble sort for comparison, and a thread count argument were
added so they could be benchmarked. The sorted check in p_bubble_1.c was
moved out of the sorting loop because it was printing on every pass.

p_bubble_1.c splits the array into one segment per thread. Each thread
bubble sorts its segment and swaps across the boundary into the next
segment. A barrier keeps the threads on the same pass.

p_bubble_2.c uses odd-even transposition sort. Even phases compare pairs
(0,1), (2,3), ... and odd phases compare pairs (1,2), (3,4), ... so no
two threads touch the same element in the same phase. A barrier is used
between each phase.

Compile:
gcc -Wall -pthread p_bubble_1.c -o p_bubble_1
gcc -Wall -pthread p_bubble_2.c -o p_bubble_2

Run:
./p_bubble_1 <number_of_threads>      (1 - 4)
./p_bubble_2 <number_of_threads>      (1 - 4)


Test Results
------------
Sequential times are the average of all runs.

Program            Sequential    1 Thread     2 Threads    3 Threads    4 Threads
Num_Integrate      2.661 s       2.732 s      1.393 s      0.958 s      0.769 s
Merge_approach_1   9.601 ms      10.252 ms    4.696 ms     3.694 ms     3.488 ms
Merge_approach_2   9.601 ms      8.782 ms     6.409 ms     5.298 ms     3.015 ms
Merge_approach_3   9.601 ms      10.226 ms    5.359 ms     4.579 ms     3.419 ms
Bubble_1           10.002 s      10.894 s     7.770 s      7.954 s      8.539 s
Bubble_2           10.083 s      5.648 s      5.328 s      6.087 s      7.108 s