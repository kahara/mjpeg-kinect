#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
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
  uint8_t * test_frame_rgb, * test_frame_ir;
  uint32_t test_counter_rgb = 0, test_counter_ir = 0;
  
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
    test_frame_rgb = malloc(SETUP_IMAGE_SIZE_RAW_RGB);
    memset(test_frame_rgb, 0, SETUP_IMAGE_SIZE_RAW_RGB);
  }
  
  if(SETUP_STREAMS & SETUP_STREAM_IR) {
    test_frame_ir = malloc(SETUP_IMAGE_SIZE_RAW_IR);
    memset(test_frame_ir, 0, SETUP_IMAGE_SIZE_RAW_IR);
  }
  
  while(1) {
    printf("get_or_set_grab_request(): %llu\n", (unsigned long long int)get_or_set_grab_request(0));
    
    tv.tv_sec = 0;
    tv.tv_usec = SETUP_POLL_DELAY;
    select(0, NULL, NULL, NULL, &tv);
    
    if(grab_new_frame) {
      grab_new_frame = 0;
      
      test_frame_rgb[test_counter_rgb] = 255;
      test_frame_ir[test_counter_ir] = 255;
      
      if(sem_trywait(&output->empty)) {
#ifdef DEBUG
	printf("grabber dropping frame\n");
#endif
      } else {
	pthread_mutex_lock(&output->lock);
#ifdef DEBUG
	printf("grabber producing new frame\n");
#endif
	output->serial++;
	
	if(SETUP_STREAMS & SETUP_STREAM_RGB) {
	  output->rgb[output->serial % SETUP_BUFFER_LENGTH_G2P].size = SETUP_IMAGE_SIZE_RAW_RGB;
	  memcpy(output->rgb[output->serial % SETUP_BUFFER_LENGTH_G2P].data, test_frame_rgb, SETUP_IMAGE_SIZE_RAW_RGB);
	}
	
	if(SETUP_STREAMS & SETUP_STREAM_IR) {
	  output->ir[output->serial % SETUP_BUFFER_LENGTH_G2P].size = SETUP_IMAGE_SIZE_RAW_IR;
	  memcpy(output->ir[output->serial % SETUP_BUFFER_LENGTH_G2P].data, test_frame_ir, SETUP_IMAGE_SIZE_RAW_IR);	  
	}
	
	sem_post(&output->full);
	pthread_mutex_unlock(&output->lock);
      }
      
    }
  }
  
  return NULL;
}
