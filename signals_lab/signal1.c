#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

static volatile sig_atomic_t got_alarm = 0;

void alarm_handler(int sig) {
  (void)sig;
  printf("Hello World!\n");
  got_alarm = 1;
}

int main(void) {
  if (signal(SIGALRM, alarm_handler) == SIG_ERR) {
    perror("signal");
    return 1;
  }
  alarm(5);                 // after 5 seconds, SIGALRM
  while (!got_alarm) {      // busy wait for simplicity (as in the assignment)
    /* spin */
  }
  printf("Turing was right!\n");
  return 0;
}
