Assignment 3 - From Pthreads to OpenMP
CSC 410 Parallel Computing, Fall 2026 - Keaton Hagen

Each problem has a sequential version (from Assignment 1), a pthread
version, and an OpenMP version. Only the computation is timed:
CLOCK_MONOTONIC for sequential/pthread, omp_get_wtime() for OpenMP.

Files:
  Task 1 Array Sum:       arraysum.c, sumT.c, omp_sum.c
  Task 2 Matrix Multiply: mainM.c + matrixMul.h, matrixT.c + matrixT.h, omp_matrix.c
  Task 3 Trapezoidal:     numIntegrate.c, trapT.c, omp_trap.c
  Task 4 N Queens:        mainQ.c, nqueensT.c, omp_nqueens.c (all use nqueens.h)

Changes to the Assignment 1 code:
  - Sizes raised to the Assignment 3 values (array N = 1,000,000,000,
    integration intervals = 1,000,000,000, N Queens N = 15).
  - Array sum uses a malloc'd array and a long long sum (int would overflow).
  - N Queens counts solutions instead of printing them, so the timing
    measures solving, not printing 2,279,184 lines.

Requirements: GCC with OpenMP, Ubuntu/WSL. The array sum needs about 4 GB of RAM.

Compile:
  gcc -Wall -Wextra -O2 arraysum.c -o arraysum
  gcc -Wall -Wextra -O2 mainM.c -o mainM
  gcc -Wall -Wextra -O2 numIntegrate.c -o numIntegrate
  gcc -Wall -Wextra -O2 mainQ.c -o mainQ

  gcc -Wall -Wextra -O2 sumT.c -o sumT -pthread
  gcc -Wall -Wextra -O2 matrixT.c -o matrixT -pthread
  gcc -Wall -Wextra -O2 trapT.c -o trapT -pthread
  gcc -Wall -Wextra -O2 nqueensT.c -o nqueensT -pthread

  gcc -Wall -Wextra -O2 -fopenmp omp_sum.c -o omp_sum
  gcc -Wall -Wextra -O2 -fopenmp omp_matrix.c -o omp_matrix
  gcc -Wall -Wextra -O2 -fopenmp omp_trap.c -o omp_trap
  gcc -Wall -Wextra -O2 -fopenmp omp_nqueens.c -o omp_nqueens

Run (parallel versions take the thread count: 1, 2, 5, 7 or 9):
  ./arraysum       ./sumT 4       ./omp_sum 4
  ./mainM          ./matrixT 4    ./omp_matrix 4
  ./numIntegrate   ./trapT 4      ./omp_trap 4
  ./mainQ          ./nqueensT 4   ./omp_nqueens 4

Each program prints its result, whether it is correct, and the time.

Scheduling (Task 6): the OpenMP loops use schedule(runtime), so the
schedule is chosen with OMP_SCHEDULE (static is used if it is not set):
  OMP_SCHEDULE=static       ./omp_nqueens 4
  OMP_SCHEDULE=dynamic,1000 ./omp_nqueens 4
  OMP_SCHEDULE=guided       ./omp_nqueens 4
