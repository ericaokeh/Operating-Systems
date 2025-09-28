#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

static volatile sig_atomic_t seconds_elapsed = 0;

void alarm_handler(int sig) {
  (void)sig;
  seconds_elapsed++;        // count every second
}

void sigint_handler(int sig) {
  (void)sig;
  printf("\nTotal time running: %d seconds\n", seconds_elapsed);
  _exit(0);                 // async-signal-safe exit
}

int main(void) {
  if (signal(SIGALRM, alarm_handler) == SIG_ERR) { perror("signal SIGALRM"); return 1; }
  if (signal(SIGINT,  sigint_handler) == SIG_ERR) { perror("signal SIGINT");  return 1; }

  alarm(1);                 // first SIGALRM in 1s
  for (;;) {
    pause();                // sleep until a signal
    alarm(1);               // reschedule next second
  }
  return 0;
}
