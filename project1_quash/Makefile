CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=gnu99 -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L
TARGET = quash

all: $(TARGET)

$(TARGET): shell.c
	$(CC) $(CFLAGS) shell.c -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)