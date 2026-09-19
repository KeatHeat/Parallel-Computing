Assignment 2 - part 1

Files:
-starwars.c
-seq_sum.c
-process_sums.c
-A2_1pdf.pdf

Requirements:
-GCC Compiler
-Ubuntu Linux or WSL

Compile and run:
Star Wars Program:
gcc -Wall -Wextra -O2 starwars.c -o starwars
./starwars

Sequential sum (N = 100,000,000):
gcc -Wall -Wextra -O2 seq_sum.c -o seq_sum 
./seq_sum 100000000

Parallel sum (N = 100,000,000, 4 processes):
gcc -Wall -Wextra -O2 process_sums.c -o process_sums 
./process_sums 100000000 4

Expected sum:
5000000050000000

Process program accepts the array size and process count as a command line argument