// pipes_processes2.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX 2048

static void chomp(char *s) {
    size_t n = strlen(s);
    if (n && s[n-1] == '\n') s[n-1] = '\0';
}

int main(void) {
    int p12[2]; // parent -> child
    int p21[2]; // child  -> parent

    if (pipe(p12) == -1 || pipe(p21) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // ========== Child (P2) ==========
        close(p12[1]);  // not writing to p12
        close(p21[0]);  // not reading from p21

        char from_p1[MAX] = {0};
        ssize_t n = read(p12[0], from_p1, sizeof(from_p1)-1);
        if (n < 0) { perror("read P2"); exit(1); }
        from_p1[n] = '\0';

        // First result: input + "howard.edu"
        char combo[MAX] = {0};
        snprintf(combo, sizeof(combo), "%showard.edu", from_p1);

        printf("Other string is: howard.edu\n\n");
        printf("Input : %s\n", from_p1);
        printf("Output: %s\n\n", combo);
        fflush(stdout);

        // Ask for second input and append it
        char second[MAX] = {0};
        printf("Input : ");
        fflush(stdout);
        if (!fgets(second, sizeof(second), stdin)) {
            fprintf(stderr, "Failed to read second input\n");
            exit(1);
        }
        chomp(second);
        strncat(combo, second, sizeof(combo) - strlen(combo) - 1);

        // Send combined string back to parent
        size_t to_send = strlen(combo) + 1;
        if (write(p21[1], combo, to_send) != (ssize_t)to_send) {
            perror("write P2");
            exit(1);
        }

        close(p12[0]);
        close(p21[1]);
        _exit(0);
    } else {
        // ========== Parent (P1) ==========
        close(p12[0]);  // not reading from p12
        close(p21[1]);  // not writing to p21

        char first[MAX] = {0};
        printf("Input : ");
        fflush(stdout);
        if (!fgets(first, sizeof(first), stdin)) {
            fprintf(stderr, "Failed to read first input\n");
            return 1;
        }
        chomp(first);

        size_t to_send = strlen(first) + 1;
        if (write(p12[1], first, to_send) != (ssize_t)to_send) {
            perror("write P1");
            return 1;
        }
        close(p12[1]); // important: signal EOF to child

        char back[MAX] = {0};
        ssize_t n = read(p21[0], back, sizeof(back)-1);
        if (n < 0) { perror("read P1"); return 1; }
        back[n] = '\0';
        close(p21[0]);

        strncat(back, "gobison.org", sizeof(back) - strlen(back) - 1);
        printf("Output: %s\n", back);
        fflush(stdout);

        wait(NULL);
    }

    return 0;
}
