CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -O2

all: signal1 signal2 timer

signal1: signal1.c
$(CC) $(CFLAGS) $< -o $@

signal2: signal2.c
$(CC) $(CFLAGS) $< -o $@

timer: timer.c
$(CC) $(CFLAGS) $< -o $@

clean:
rm -f signal1 signal2 timer *.o
.PHONY: all clean
