#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>      // fork(), sleep(), getpid(), getppid()
#include <sys/wait.h>    // wait()

void child_process();

int main(void) {
    int pid;

    // Create two child processes
    for (int i = 0; i < 2; i++) {
        pid = fork();
        if (pid == 0) {                 // child
            child_process();            // never returns
        } else if (pid < 0) {           // fork failed
            perror("fork");
            exit(1);
        }
    }

    // Parent waits for both children
    for (int i = 0; i < 2; i++) {
        int status;
        int completed_pid = wait(&status);
        printf("Child Pid: %d has completed with exit status: %d\n",
               completed_pid, WEXITSTATUS(status));
    }
    return 0;
}

void child_process() {
    int pid = getpid();
    int parent_pid = getppid();

    srandom(pid);                        // seed PRNG
    int n = 1 + (random() % 30);          // loop up to 30 times

    for (int i = 0; i < n; i++) {
        int t = 1 + (random() % 10);      // sleep 1–10 seconds
        printf("Child Pid: %d is going to sleep for %d seconds!\n", pid, t);
        sleep(t);
        printf("Child Pid: %d is awake!\nWhere is my Parent: %d?\n",
               pid, parent_pid);
    }
    exit(0);
}
