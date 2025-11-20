# Define the name of the executable
EXECUTABLE = my3proc

# Define the source file
SOURCE = main3.c

# Define the compiler and flags
CC = gcc
CFLAGS = -Wall -Werror -std=c99

# The default target: builds the executable
all: $(EXECUTABLE)

# Rule to compile the source file into the executable
$(EXECUTABLE): $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) -o $(EXECUTABLE)

# Rule to clean up generated files
clean:
	rm -f $(EXECUTABLE)