// pipes_processes3.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int pipe1[2]; // Between cat and grep
    int pipe2[2]; // Between grep and sort

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <search_term>\n", argv[0]);
        exit(1);
    }

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        exit(1);
    }

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        exit(1);
    }

    if (pid1 == 0) {
        // ===== First child: executes "cat scores" =====
        dup2(pipe1[1], STDOUT_FILENO); // stdout -> pipe1 write end
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        execlp("cat", "cat", "scores", NULL);
        perror("execlp cat");
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        exit(1);
    }

    if (pid2 == 0) {
        // ===== Second child: executes "grep <arg>" =====
        dup2(pipe1[0], STDIN_FILENO);   // read from pipe1
        dup2(pipe2[1], STDOUT_FILENO);  // write to pipe2
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        execlp("grep", "grep", argv[1], NULL);
        perror("execlp grep");
        exit(1);
    }

    pid_t pid3 = fork();
    if (pid3 < 0) {
        perror("fork");
        exit(1);
    }

    if (pid3 == 0) {
        // ===== Third child: executes "sort" =====
        dup2(pipe2[0], STDIN_FILENO); // read from pipe2
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        execlp("sort", "sort", NULL);
        perror("execlp sort");
        exit(1);
    }

    // ===== Parent process =====
    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);

    // Wait for all children
    wait(NULL);
    wait(NULL);
    wait(NULL);

    return 0;
}
