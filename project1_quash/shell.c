// ------------------------------------------------------------
// Project 1 - Quash Shell
// Student: Your Name(s)
// ------------------------------------------------------------

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128

extern char **environ;

/* -------- prompt with cwd -------- */
static void print_prompt(void) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd))) {
        printf("%s> ", cwd);
    } else {
        printf("> ");
    }
    fflush(stdout);
}

/* -------- SIGINT handling -------- */
static volatile sig_atomic_t got_sigint = 0;
static void sigint_handler(int sig) {
    (void)sig;
    got_sigint = 1;
    ssize_t _n = write(STDOUT_FILENO, "\n", 1);
    (void)_n; // silence unused-return warning
}

/* -------- util: expand $VARS in token -------- */
static char *expand_vars(const char *s) {
    size_t n = strlen(s);
    char *out = malloc(n * 2 + 1);
    if (!out) return strdup(s);
    size_t oi = 0;
    for (size_t i = 0; i < n;) {
        if (s[i] == '$') {
            i++;
            size_t start = i;
            while (i < n && (isalnum((unsigned char)s[i]) || s[i] == '_')) i++;
            size_t len = i - start;
            if (len == 0) { out[oi++] = '$'; continue; }
            char name[256];
            if (len >= sizeof(name)) len = sizeof(name) - 1;
            memcpy(name, s + start, len);
            name[len] = '\0';
            const char *val = getenv(name);
            if (!val) val = "";
            size_t vl = strlen(val);
            memcpy(out + oi, val, vl);
            oi += vl;
        } else {
            out[oi++] = s[i++];
        }
    }
    out[oi] = '\0';
    return out;
}

/* -------- tokenizer -------- */
static int tokenize(const char *line, char *argv[], int maxv) {
    int argc = 0;
    const char *p = line;
    while (*p) {
        while (isspace((unsigned char)*p)) p++;
        if (!*p) break;

        if (strchr("<>|&", *p)) {
            if (argc < maxv - 1) {
                char *tok = malloc(2);
                tok[0] = *p;
                tok[1] = '\0';
                argv[argc++] = tok;
            }
            p++;
            continue;
        }

        char buf[MAX_COMMAND_LINE_LEN];
        int bi = 0;
        if (*p == '"' || *p == '\'') {
            char q = *p++;
            while (*p && *p != q) buf[bi++] = *p++;
            if (*p == q) p++;
        } else {
            while (*p && !isspace((unsigned char)*p) && !strchr("<>|&", *p))
                buf[bi++] = *p++;
        }
        buf[bi] = '\0';

        char *expanded = expand_vars(buf);
        if (argc < maxv - 1)
            argv[argc++] = expanded;
        else
            free(expanded);
    }
    argv[argc] = NULL;
    return argc;
}

static void free_argv(char *argv[]) {
    for (int i = 0; argv[i]; ++i) free(argv[i]);
}

/* -------- built-ins -------- */
static bool is_builtin(const char *cmd) {
    return cmd &&
           (!strcmp(cmd, "cd") || !strcmp(cmd, "pwd") || !strcmp(cmd, "echo") ||
            !strcmp(cmd, "env") || !strcmp(cmd, "setenv") || !strcmp(cmd, "exit"));
}

static int bi_cd(char *argv[]) {
    const char *dir = argv[1] ? argv[1] : getenv("HOME");
    if (!dir) dir = "/";
    if (chdir(dir) != 0) { perror("cd"); return 1; }
    return 0;
}

static int bi_pwd(void) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd))) { puts(cwd); return 0; }
    perror("pwd");
    return 1;
}

static int bi_echo(char *argv[]) {
    for (int i = 1; argv[i]; ++i) {
        fputs(argv[i], stdout);
        if (argv[i + 1]) fputc(' ', stdout);
    }
    fputc('\n', stdout);
    return 0;
}

static int bi_env(char *name) {
    if (name) { const char *v = getenv(name); if (v) puts(v); return 0; }
    for (char **e = environ; e && *e; ++e) puts(*e);
    return 0;
}

static int bi_setenv(char *argv[]) {
    if (!argv[1]) { fprintf(stderr, "usage: setenv NAME value | NAME=value\n"); return 1; }
    char name[256], value[MAX_COMMAND_LINE_LEN];
    char *eq = strchr(argv[1], '=');
    if (eq) {
        size_t nl = (size_t)(eq - argv[1]);
        if (nl >= sizeof(name)) nl = sizeof(name) - 1;
        memcpy(name, argv[1], nl);
        name[nl] = '\0';
        snprintf(value, sizeof(value), "%s", eq + 1);
    } else {
        if (!argv[2]) { fprintf(stderr, "usage: setenv NAME value\n"); return 1; }
        snprintf(name, sizeof(name), "%s", argv[1]);
        snprintf(value, sizeof(value), "%s", argv[2]);
    }
    if (setenv(name, value, 1) != 0) { perror("setenv"); return 1; }
    return 0;
}

static int run_builtin(char *argv[]) {
    if (!argv[0]) return 0;
    if (!strcmp(argv[0], "cd"))    return bi_cd(argv);
    if (!strcmp(argv[0], "pwd"))   return bi_pwd();
    if (!strcmp(argv[0], "echo"))  return bi_echo(argv);
    if (!strcmp(argv[0], "env"))   return bi_env(argv[1]);
    if (!strcmp(argv[0], "setenv"))return bi_setenv(argv);
    if (!strcmp(argv[0], "exit"))  exit(0);
    return -1;
}

/* -------- plan: args + redirs + pipe + bg flag -------- */
struct plan {
    char *left[MAX_COMMAND_LINE_ARGS];
    char *right[MAX_COMMAND_LINE_ARGS];
    char *infile;
    char *outfile;
    bool has_pipe;
    bool background;
};

static void build_plan(char *argv[], struct plan *p) {
    memset(p, 0, sizeof(*p));
    int ai = 0, li = 0, ri = 0;
    bool on_right = false;
    while (argv[ai]) {
        if (!strcmp(argv[ai], "&")) p->background = true;
        else if (!strcmp(argv[ai], "<")) p->infile = argv[++ai];
        else if (!strcmp(argv[ai], ">")) p->outfile = argv[++ai];
        else if (!strcmp(argv[ai], "|")) { p->has_pipe = true; on_right = true; }
        else {
            if (!on_right) p->left[li++] = argv[ai];
            else           p->right[ri++] = argv[ai];
        }
        ai++;
    }
    p->left[li] = NULL; p->right[ri] = NULL;
}

static void apply_redirs(const struct plan *p) {
    if (p->infile) {
        int fd = open(p->infile, O_RDONLY);
        if (fd < 0) { perror("open <"); _exit(127); }
        if (dup2(fd, STDIN_FILENO) < 0) { perror("dup2 <"); _exit(127); }
        close(fd);
    }
    if (p->outfile) {
        int fd = open(p->outfile, O_CREAT | O_TRUNC | O_WRONLY, 0666);
        if (fd < 0) { perror("open >"); _exit(127); }
        if (dup2(fd, STDOUT_FILENO) < 0) { perror("dup2 >"); _exit(127); }
        close(fd);
    }
}

/* -------- run commands -------- */
static pid_t launch_simple(const struct plan *p) {
    pid_t c = fork();
    if (c < 0) { perror("fork"); return -1; }
    if (c == 0) {
        signal(SIGINT, SIG_DFL);
        apply_redirs(p);
        execvp(p->left[0], p->left);
        perror("execvp");
        _exit(127);
    }
    return c;
}

static int run_plan(struct plan *p) {
    if (!p->left[0]) return 0;

    if (!p->has_pipe && !p->background &&
        !p->infile && !p->outfile && is_builtin(p->left[0])) {
        return run_builtin(p->left);
    }

    pid_t left_pid = -1, right_pid = -1;
    int pipefd[2] = {-1, -1};
    if (p->has_pipe) {
        if (pipe(pipefd) < 0) { perror("pipe"); return 1; }

        left_pid = fork();
        if (left_pid == 0) {
            signal(SIGINT, SIG_DFL);
            if (dup2(pipefd[1], STDOUT_FILENO) < 0) { perror("dup2 pipe write"); _exit(127); }
            close(pipefd[0]); close(pipefd[1]);
            execvp(p->left[0], p->left);
            perror("execvp");
            _exit(127);
        }

        right_pid = fork();
        if (right_pid == 0) {
            signal(SIGINT, SIG_DFL);
            if (dup2(pipefd[0], STDIN_FILENO) < 0) { perror("dup2 pipe read"); _exit(127); }
            close(pipefd[0]); close(pipefd[1]);
            execvp(p->right[0], p->right);
            perror("execvp");
            _exit(127);
        }

        close(pipefd[0]); close(pipefd[1]);
    } else {
        left_pid = launch_simple(p);
    }

    if (p->background) {
        printf("[bg %d]\n", left_pid);
        return 0;
    }

    got_sigint = 0;
    time_t start = time(NULL);
    int status;

    while (1) {
        pid_t w = waitpid(p->has_pipe ? -1 : left_pid, &status, WNOHANG);
        if (w > 0) break;
        if (w == 0) {
            if (got_sigint) {
                kill(left_pid, SIGINT);
                if (p->has_pipe && right_pid > 0) kill(right_pid, SIGINT);
                got_sigint = 0;
            }
            if (difftime(time(NULL), start) >= 10.0) {
                kill(left_pid, SIGKILL);
                if (p->has_pipe && right_pid > 0) kill(right_pid, SIGKILL);
                break;
            }
            usleep(10000);
            continue;
        }
        if (w < 0 && errno != EINTR) { perror("waitpid"); break; }
    }
    return 0;
}

/* -------- main loop -------- */
int main(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);

    char line[MAX_COMMAND_LINE_LEN];
    while (true) {
        while (waitpid(-1, NULL, WNOHANG) > 0) {}
        print_prompt();

        if (!fgets(line, sizeof(line), stdin)) { printf("\n"); break; }
        if (line[0] == '\n') continue;
        line[strcspn(line, "\r\n")] = '\0';

        char *argv[MAX_COMMAND_LINE_ARGS] = {0};
        int argc = tokenize(line, argv, MAX_COMMAND_LINE_ARGS);
        if (argc == 0) { continue; }

        struct plan p;
        build_plan(argv, &p);
        run_plan(&p);

        free_argv(argv);
        fflush(stdout);
        fflush(stderr);
    }
    return 0;
}
