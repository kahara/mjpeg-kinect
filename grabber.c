#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <inttypes.h>
#include <stdlib.h>
#include "settings.h"
#include "grabber.h"
#include "interthread.h"

/* pthread_mutex_t lock; // producer signals consumer with cv that a new frame is available */
/* pthread_cond_t new_frame; */

uint8_t grab_new_frame;
void tick(int signum) { grab_new_frame = 1; }

void * grabber(void * args)
{
  struct thread_arg * ta = (struct thread_arg *)args;
  struct channel * output = ta->output;
  struct timeval tv;
  
  // The new frame "ticker"
  struct sigaction sa;
  struct itimerval tmr;
  
  // XXX dummy test frame
  uint8_t * frame_rgb;
  
  // Prepare new frame tick handler
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = tick;
  sigaction(SIGALRM, &sa, NULL);
  
  // Set up timer to tick every SETUP_STREAM_INTERVAL milliseconds  
  // XXX When at the end of program run, stop this timer before doing anything else
  tmr.it_value.tv_sec = SETUP_STREAM_INTERVAL / 1000;
  tmr.it_value.tv_usec = (SETUP_STREAM_INTERVAL * 1000) % 1000000;
  tmr.it_interval = tmr.it_value;
  setitimer(ITIMER_REAL, &tmr, NULL);
  
  if(SETUP_STREAMS & SETUP_STREAM_RGB) {
    frame_rgb = malloc(SETUP_IMAGE_SIZE_RAW_RGB);
  }
  
  while(1) {
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    select(0, NULL, NULL, NULL, &tv);
    
    if(grab_new_frame) {
      grab_new_frame = 0;
      
      if(sem_trywait(&output->empty)) {
	printf("grabber dropping frame\n");
      } else {
	pthread_mutex_lock(&output->lock);
	printf("grabber producing new frame\n");
	sem_post(&output->full);
	pthread_mutex_unlock(&output->lock);
      }
      
    }
  }
  
  return NULL;
}
