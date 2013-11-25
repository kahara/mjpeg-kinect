#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <inttypes.h>
#include <stdlib.h>
#include "settings.h"
#include "grabber.h"
#include "interthread.h"

void * grabber(void * args)
{
  struct thread_arg * ta = (struct thread_arg *)args;
  struct channel * input = ta->input, * output = ta->output;
  
  uint8_t * frame_rgb; // XXX dummy test frame
  
  if(SETUP_STREAMS & SETUP_STREAM_RGB) {
    frame_rgb = malloc(SETUP_IMAGE_SIZE_RAW_RGB);
  }
  
  while(1) {
    pthread_mutex_lock(&input->lock);
    pthread_cond_wait(&input->new_frame, &input->lock);
    pthread_mutex_unlock(&input->lock);
    
    printf("grab new frame\n");
    
    pthread_mutex_lock(&output->lock);
    pthread_cond_signal(&output->new_frame);
    pthread_mutex_unlock(&output->lock);
  }
  
  return NULL;
}
