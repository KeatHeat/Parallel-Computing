Assignment 2 - Part 2: Multithreading with Pthreads

This assignment converts the array sum and matrix multiplication programs
from Assignment 1 into multithreaded programs using POSIX threads (pthreads).

Files:
- sumT.c - Parallel array sum using pthreads
- matrixT.c - Parallel matrix multiplication using pthreads
- matrixT.h - Header file for matrix multiplication
- seq_sum.c - Sequential array sum used for benchmarking
- A2_Part_2.pdf - Task 3 benchmark results and Task 4 observations

Compilation:
gcc -Wall -Wextra -O2 seq_sum.c -o seq_sum
gcc -Wall -Wextra -O2 sumT.c -o sumT -pthread
gcc -Wall -Wextra -O2 matrixT.c -o matrixT -pthread

Running:
./seq_sum 100000000
./sumT 1
./sumT 2
./sumT 3
./sumT 4

./matrixT 1
./matrixT 2
./matrixT 3
./matrixT 4

The parallel programs use pthreads to divide the work among multiple
threads. The array sum assigns each thread a portion of the array, while
matrix multiplication assigns each thread a portion of the result matrix.