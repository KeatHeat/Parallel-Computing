Assignment 2 - Part 3
Synchronization of Threads

Name: Keaton Hagen
Course: CSC 410 Parallel Computing
Semester: Fall 2026

Task 1 - neuromancer.c
----------------------
This program uses a mutex and a condition variable to synchronize the
player threads.

The mutex protects the shared game state, including the current player,
game state, and scores. The condition variable makes each player wait
until it is their turn.

The players take turns in order: Player 1, Player 2, Player 3, and then
the cycle repeats. The game runs for 10 seconds before ending.

Compile:
gcc -Wall -Wextra -O2 neuromancer.c -o neuromancer -pthread

Run:
./neuromancer


Task 2 - average.c
-------------------
This program uses a pthread barrier to synchronize the five threads.

Each thread calculates the partial sum for its section of the array.
The barrier makes all of the threads wait until everyone has finished
calculating their partial sum before Thread 0 calculates the total and
average.

The final total is 210 and the average is 10.50.

Compile:
gcc -Wall -Wextra -O2 average.c -o average -pthread

Run:
./average


Task 3 - paradox.c
------------------
This program compares two different ways of storing the sums while
multiple threads are running.

Run A stores the sums next to each other in memory. Run B separates the
sums using extra space so that the values are placed on separate cache
lines.

The tests were run using 1, 2, 4, 8, and 12 threads.

Run A became significantly slower as more threads were added, while
Run B stayed around 0.23 seconds. This is related to false sharing and
cache-line contention.

The program also produces inconsistent sum results because the threads
update shared values without synchronization. The volatile keyword does
not make the += operation thread-safe.

A better design would have each thread calculate its sum using a local
variable and only write the final result once the thread is finished.
The main thread could then combine the results after all threads have
finished.


Test Results
------------
Threads    Run A       Run B
1          0.2327 s    0.2273 s
2          0.9258 s    0.2351 s
4          2.1061 s    0.2247 s
8          4.2351 s    0.2267 s
12         4.1675 s    0.2349 s
