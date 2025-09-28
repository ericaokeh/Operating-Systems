#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

static volatile sig_atomic_t tick = 0;

void alarm_handler(int sig) {
  (void)sig;
  printf("Hello World!\n");
  tick = 1;                 // tell main a tick occurred
}

int main(void) {
  if (signal(SIGALRM, alarm_handler) == SIG_ERR) {
    perror("signal");
    return 1;
  }
  alarm(5);                 // first tick in 5s
  for (;;) {
    pause();                // sleep until any signal arrives
    if (tick) {
      tick = 0;
      printf("Turing was right!\n");
      alarm(5);             // schedule the next tick
    }
  }
  return 0;
}
