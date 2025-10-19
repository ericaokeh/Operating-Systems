# Project 1 - Quash Shell

Student: Erica O.  
Course: Operating Systems  
Environment: Codio / Linux (GCC, POSIX system calls)

---

## Submission Instructions

Push your code to GitHub and post the GitHub URL in Google Classroom as your submission.  
You must include a report (at least two pages) as part of this README that explains your design choices and documents your code.  
Include your name (and your teammate’s name, if you have one) in all source code and here in this README.

---

## How to Build and Run

make # build the shell
./quash # run it
make clean # remove compiled files

yaml
Copy code

This shell was developed and tested in Codio using Linux and gcc.

---

## Project Overview

This project implements a simple Unix-style command shell called Quash.  
It supports basic built-in commands, process creation, environment variables, background jobs, signal handling, and simple input/output redirection and pipes.

---

## Features Implemented

- Custom prompt that shows the current working directory  
- Command parsing and tokenization with support for quoted text  
- Built-in commands: cd, pwd, echo, env, setenv, and exit  
- Environment variable expansion (e.g., $HOME, $USER)  
- Execution of external commands using fork() and execvp()  
- Background job support using &  
- Signal handling so Ctrl-C interrupts only the running process, not the shell  
- Automatic kill of long-running foreground jobs after 10 seconds  
- Input/output redirection using < and >  
- Simple single pipe support using |

---

## Design and Implementation

### Prompt
The shell uses getcwd() to obtain the current working directory and prints it followed by ">".  
This provides clear context for the user’s current location.

### Tokenization
The command line is split into tokens using whitespace, while respecting quoted strings.  
Special tokens like <, >, |, and & are detected separately.  
If a token begins with $, the shell expands it using getenv() to replace environment variables with their values.

### Built-in Commands
- cd [dir] changes the directory. If no directory is provided, it changes to HOME.  
- pwd prints the current working directory.  
- echo prints its arguments to standard output.  
- env [VAR] prints one variable or all environment variables.  
- setenv NAME VALUE sets or updates an environment variable.  
- exit exits the shell process.

### External Commands
If a command is not one of the built-ins, the shell creates a new process with fork() and executes the command using execvp().  
If execvp() fails, an error message is printed with perror().

### Background Jobs
When a command ends with &, the shell runs it in the background without waiting for it to finish.  
The PID of the background process is printed, and the shell immediately shows a new prompt.  
Completed background jobs are cleaned up using waitpid() with WNOHANG.

### Signal Handling
A custom SIGINT handler allows Ctrl-C to interrupt only the running process and not the shell itself.  
This prevents Quash from quitting when a foreground command is interrupted.

### Timeout
Foreground processes are monitored for execution time.  
If a command runs longer than 10 seconds, the shell sends SIGKILL to terminate it.  
Background processes are not affected by the timeout.

### Redirection and Pipes
- < filename redirects input from a file.  
- > filename redirects output to a file (creates or overwrites).  
- cmd1 | cmd2 connects two commands so that the output of the first is used as input for the second.

---

## Example Session

/home/codio/workspace> echo hello world
hello world

/home/codio/workspace> setenv greeting=hello
/home/codio/workspace> echo $greeting $HOME
hello /home/codio

/home/codio/workspace> cd testDir1
/home/codio/workspace/testDir1> pwd
/home/codio/workspace/testDir1

/home/codio/workspace/testDir1> ls | grep file
file1.txt
file2.txt

/home/codio/workspace> cat shell.c > output.txt
/home/codio/workspace> more < output.txt

/home/codio/workspace> sleep 20

After 10 seconds, the shell automatically kills the process.
yaml
Copy code

---

## Testing Summary

- All built-ins (cd, pwd, echo, env, setenv, exit) work correctly  
- Environment variable expansion works  
- External commands run normally  
- Background jobs work and return immediately  
- Ctrl-C interrupts the foreground process only  
- Long-running processes are killed after 10 seconds  
- Redirection and single-pipe commands work correctly

---

## Known Limitations

- Only one pipe is supported (no cmd1 | cmd2 | cmd3)  
- No append redirection (>>)  
- No job control commands such as fg, bg, or jobs  
- Limited handling of escaped characters in input  

---

## Lessons Learned

- Learned how to use fork(), execvp(), and waitpid() for process management  
- Understood how signals and SIGINT work in Linux  
- Practiced file descriptor manipulation for redirection and pipes  
- Improved understanding of how a shell interacts with the operating system  

---

## References

- Linux man pages: fork, execvp, waitpid, kill, signal, dup2, pipe, chdir, getcwd, perror  
- Operating Systems course labs and textbook  
- ChatGPT (for writing help and formatting)
