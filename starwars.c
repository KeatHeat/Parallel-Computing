
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Starting shield power level
int shield_power = 50;

int main() {
    pid_t pid;
    int status;

    printf("Millennium Falcon: Initial shield power level: %d%%\n\n", shield_power);
    fflush(stdout);

    // Create 4 child processes - 4 different characters adjusting shield power

    // Luke increases the shield power by 25
    // Han increases the shield power by 20
    // Chewbacca increases the shield power by 30
    // Leia increases the shield power by 15

    char *names[] = {"Luke", "Han", "Chewbacca", "Leia"};
    int increases[] = {25, 20, 30, 15};

    for (int i = 0; i < 4; i++) {
        pid = fork();

        // Check if process creation failed
        if (pid < 0) {
            perror("fork failed");
            return 1;
        }

        if (pid == 0) {
            printf("%s: Adjusting shield power\n", names[i]);

            shield_power += increases[i];

            printf("%s: Shield power level now at %d%%\n",
                   names[i], shield_power);

            return 0;
        }
    }

    // Make parent process wait for all child processes to complete
    for (int i = 0; i < 4; i++) {
        wait(&status);
    }

    // Parent process reports final state
    printf("\nFinal shield power level on the Millennium Falcon: %d%%\n",
           shield_power);

    printf("\nMay the forks be with you!\n");
    return 0;
}