# Lab 5 — Function Pointers (Single-file submission)

This file contains the completed solutions for:
- Task 1: `my_comparer()` for sorting processes (priority DESC, arrival ASC)
- Task 2: function-pointer calculator without conditionals + safe invalid-input handling

---

## Task 1 — func-ptr.c (my_comparer)

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "process.h"
#include "util.h"

#define DEBUG 0

typedef int (*Comparer)(const void *a, const void *b);

int my_comparer(const void *this, const void *that)
{
    const Process *a = (const Process *)this;
    const Process *b = (const Process *)that;

    if (a->priority != b->priority) {
        return b->priority - a->priority;      // priority DESC
    }
    return a->arrival_time - b->arrival_time;  // arrival_time ASC
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <input_csv>\n", argv[0]);
        return 1;
    }

    int count = 0;
    Process *processes = load_processes(argv[1], &count);
    if (!processes) {
        perror("load_processes");
        return 1;
    }

    sort_processes(processes, count, my_comparer);
    print_processes(processes, count);

    free(processes);
    return 0;
}



