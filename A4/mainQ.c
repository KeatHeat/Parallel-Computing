// mainQ.c - Sequential N Queens from Assignment 1 (baseline for Task 4)
//
// Compile: gcc -Wall -Wextra -O2 mainQ.c -o mainQ
// Run:     ./mainQ [N]     (default N = 15)

#include "nqueens.h"
#include <time.h>

// Current time in seconds, using CLOCK_MONOTONIC as the assignment asks
static double now_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(int argc, char *argv[])
{
    int n = (argc > 1) ? atoi(argv[1]) : 15;
    if (n < 1 || n > MAXN) {
        fprintf(stderr, "N must be between 1 and %d.\n", MAXN);
        return 1;
    }

    int *board = malloc(n * sizeof(int));
    if (board == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    for (int i = 0; i < n; i++) {
        board[i] = -1;
    }

    double start = now_seconds();
    long long count = solveNQueensUtil(board, 0, n);
    double end = now_seconds();

    printf("Sequential N Queens, N = %d\n", n);
    printf("Solutions = %lld", count);
    if (n <= KNOWN_MAX) printf(" (%s)", count == KNOWN[n] ? "correct" : "WRONG");
    printf("\nTime taken: %f seconds\n", end - start);

    free(board);
    return 0;
}
