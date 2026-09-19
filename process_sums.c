
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <N> <processes>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int num_processes = atoi(argv[2]);

    if (N <= 0 || num_processes < 1 || num_processes > 4) {
        fprintf(stderr, "N must be positive and processes must be 1-4.\n");
        return 1;
    }

    int *arr = malloc((size_t)N * sizeof(int));
    if (!arr) {
        perror("malloc");
        return 1;
    }

    for (int i = 0; i < N; i++) {
        arr[i] = i + 1;
    }

    int pipes[4][2];
    pid_t pids[4];
    long long total_sum = 0;
    int children_created = 0;

    // Create a pipe for each child
    for (int i = 0; i < num_processes; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            free(arr);
            return 1;
        }
    }

    int chunk_size = N / num_processes;

    // Create child processes
    for (int i = 0; i < num_processes; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            break;
        }

        if (pid == 0) {
            // Child process
            for (int j = 0; j < num_processes; j++) {
                close(pipes[j][0]);

                if (j != i) {
                    close(pipes[j][1]);
                }
            }

            int start = i * chunk_size;
            int end = (i == num_processes - 1)
                          ? N
                          : start + chunk_size;

            long long partial_sum = 0;

            for (int j = start; j < end; j++) {
                partial_sum += arr[j];
            }

            if (write(pipes[i][1], &partial_sum,
                      sizeof(partial_sum)) != sizeof(partial_sum)) {
                perror("write");
                exit(1);
            }

            close(pipes[i][1]);
            free(arr);
            exit(0);
        }
        
        pids[i] = pid;
        children_created++;
    }

    // Parent closes all write ends
    for (int i = 0; i < num_processes; i++) {
        close(pipes[i][1]);
    }

    // Parent reads partial sums
    for (int i = 0; i < children_created; i++) {
        long long partial_sum;

        if (read(pipes[i][0], &partial_sum,
                 sizeof(partial_sum)) == sizeof(partial_sum)) {
            total_sum += partial_sum;
        } else {
            fprintf(stderr, "Failed to read partial sum.\n");
        }

        close(pipes[i][0]);
    }

    // Parent waits for children
    for (int i = 0; i < children_created; i++) {
        waitpid(pids[i], NULL, 0);
    }

    printf("Total sum = %lld\n", total_sum);

    free(arr);
    return 0;
}