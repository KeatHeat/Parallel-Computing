// nqueens.h - N Queens solver, based on the Assignment 1 sequential code.
//
// board[col] = row of the queen in that column (-1 = empty).
// Change from Assignment 1: instead of printing every solution (2,279,184
// lines for N = 15, which would make the timing measure printing, not
// solving), solveNQueensUtil now returns how many solutions it found.
#ifndef NQUEENS_H
#define NQUEENS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAXN 20

// Can a queen go at (row, col) given the queens in columns 0..col-1?
static inline bool isSafe(int board[], int row, int col)
{
    for (int i = 0; i < col; i++) {
        if (board[i] == row || abs(board[i] - row) == abs(i - col)) {
            return false;
        }
    }
    return true;
}

// Count all solutions, given queens already placed in columns 0..col-1
static long long solveNQueensUtil(int board[], int col, int n)
{
    if (col == n) {
        return 1;   // every column has a queen: one solution
    }

    long long count = 0;
    for (int row = 0; row < n; row++) {
        if (isSafe(board, row, col)) {
            board[col] = row;
            count += solveNQueensUtil(board, col + 1, n);
            board[col] = -1;   // backtrack
        }
    }
    return count;
}

// Work unit for the parallel versions: fix the queens in columns 0 and 1
// (task t puts column 0's queen in row t / n and column 1's in row t % n),
// then count every solution from column 2 on. That gives n*n = 225 tasks
// for N = 15, plenty to spread across 9 threads. Each task uses its own
// board array, so threads never share data.
static inline long long solve_task(int n, int t)
{
    int board[MAXN];
    for (int i = 0; i < n; i++) {
        board[i] = -1;
    }

    board[0] = t / n;
    if (!isSafe(board, t % n, 1)) {
        return 0;   // the first two queens attack each other
    }
    board[1] = t % n;
    return solveNQueensUtil(board, 2, n);
}

// Known answers, used to check results
static const long long KNOWN[] = {1, 1, 0, 0, 2, 10, 4, 40, 92, 352, 724, 2680,
                                  14200, 73712, 365596, 2279184, 14772512};
#define KNOWN_MAX 16

#endif
