#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "process.h"
#include "util.h"

#define DEBUG 0 // change this to 1 to enable verbose output

/**
 * Signature for an function pointer that can compare
 * You need to cast the input into its actual
 * type and then compare them according to your
 * custom logic
 */
typedef int (*Comparer)(const void *a, const void *b);

/**
 * compares 2 processes
 * You can assume:
 *  -- Process ids will be unique
 *  -- No 2 processes will have same arrival time
 *
 * Sort order (matches sample output):
 *  1) priority DESC (higher priority first)
 *  2) arrival_time ASC (earlier arrival first)
 */
int my_comparer(const void *this, const void *that)
{
    const Process *a = (const Process *)this;
    const Process *b = (const Process *)that;

    if (a->priority != b->priority) {
        return b->priority - a->priority;     // DESC by priority
    }
    return a->arrival_time - b->arrival_time; // ASC by arrival time
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <input_csv>\n", argv[0]);
        return 1;
    }

    // Provided by the lab scaffolding (process/util)
    int count = 0;
    Process *processes = load_processes(argv[1], &count);
    if (!processes) {
        perror("load_processes");
        return 1;
    }

    // Provided sort wrapper uses your comparer
    sort_processes(processes, count, my_comparer);

    // Provided printing utility
    print_processes(processes, count);

    free(processes);
    return 0;
}
