#define _XOPEN_SOURCE 600 // Required for srandom() and random() on some Linux/GNU systems

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

// Function for the child process logic
void child_task(int child_num) {
    // Seed the random number generator using the current process ID and time
    srandom((unsigned int)getpid() * (unsigned int)time(NULL));

    // Determine a random number of iterations (1 to 30)
    // Formula: (random() % (MAX - MIN + 1)) + MIN
    long num_iterations = (random() % 30) + 1;

    pid_t current_pid = getpid();
    pid_t parent_ppid = getppid();

    printf("--- Child %d (PID: %d, PPID: %d) starting %ld iterations ---\n",
           child_num, current_pid, parent_ppid, num_iterations);

    for (long i = 0; i < num_iterations; i++) {
        // Determine a random sleep time (1 to 10 seconds)
        unsigned int sleep_time = (unsigned int)(random() % 10) + 1;

        printf("Child Pid: %d is going to sleep! (Iteration %ld/%ld, Sleep: %u sec)\n",
               current_pid, i + 1, num_iterations, sleep_time);

        sleep(sleep_time); // Call sleep()

        printf("Child Pid: %d is awake!\nWhere is my Parent: %d?\n",
               current_pid, parent_ppid);
    }

    // Terminate the child process and return an exit status of 0 (success)
    printf("--- Child %d (PID: %d) finished and exiting ---\n", child_num, current_pid);
    exit(0);
}

int main(void) {
    pid_t pid1, pid2;
    // Removed unused variables status1 and status2

    // 1. Fork the first child process
    pid1 = fork();

    if (pid1 < 0) {
        perror("fork 1 failed");
        exit(1);
    } else if (pid1 == 0) {
        // Code executed by the first child
        child_task(1);
        exit(0);
    }

    // 2. Fork the second child process
    pid2 = fork();

    if (pid2 < 0) {
        perror("fork 2 failed");
        exit(1);
    } else if (pid2 == 0) {
        // Code executed by the second child
        child_task(2);
        exit(0);
    }

    // Code executed only by the parent process
    printf("Parent (PID: %d) forked two children: Child 1 (PID: %d) and Child 2 (PID: %d).\n",
           getpid(), pid1, pid2);
    printf("Parent is now waiting for both children to complete...\n");

    // 3. Parent waits for both children to complete
    pid_t terminated_pid;
    int status; // Status variable for wait()

    // Loop twice to wait for two children
    for (int i = 0; i < 2; i++) {
        terminated_pid = wait(&status); // Wait for the next child

        if (terminated_pid > 0) {
            // Check if the child exited normally
            if (WIFEXITED(status)) {
                printf("Parent: Child Pid: %d has completed with exit status %d.\n",
                       terminated_pid, WEXITSTATUS(status));
            } else {
                printf("Parent: Child Pid: %d terminated abnormally.\n", terminated_pid);
            }
        } else if (terminated_pid == -1) {
            // Should not happen if there are exactly two children and no error, but handles safety
            perror("wait error");
            break;
        }
    }

    printf("Parent (PID: %d) is done waiting and is now exiting.\n", getpid());

    return 0; // Parent process terminates
}