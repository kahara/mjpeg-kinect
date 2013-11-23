#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/select.h>
#include "settings.h"
#include "grabber.h"
#include "preprocessor.h"
#include "compressor.h"
#include "server.h"

void tick(int signum)
{
  printf("tick\n");
}

int main(int argc, char **argv)
{
  pthread_t grabber_thread, preprocessor_thread, compressor_thread, server_thread;
  struct sigaction sa;
  struct itimerval tmr;
  struct timeval tv;
  
  pthread_create(&grabber_thread, NULL, &grabber, NULL);
  pthread_create(&preprocessor_thread, NULL, &preprocessor, NULL);
  pthread_create(&compressor_thread, NULL, &compressor, NULL);
  pthread_create(&server_thread, NULL, &server, NULL);
  
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = tick;
  sigaction(SIGALRM, &sa, NULL);
  
  tmr.it_value.tv_sec = SETUP_STREAM_INTERVAL / 1000;
  tmr.it_value.tv_usec = (SETUP_STREAM_INTERVAL * 1000) % 1000000;
  tmr.it_interval = tmr.it_value;
  
  setitimer(ITIMER_REAL, &tmr, NULL);
  
  while(1) { // Do nothing; use select(2) here because sleep/usleep etc. rely on SIGALRM (as we do, see above)
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    select(0, NULL, NULL, NULL, &tv);
    printf("main\n");
  }
  
  return 0;
}
