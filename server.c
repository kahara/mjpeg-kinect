#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "settings.h"
#include "server.h"
#include "interthread.h"

void * server(void * args)
{
  struct thread_arg * ta = (struct thread_arg *)args;
  struct channel * input = ta->input;
  struct timeval tv;
  uint8_t * ibuf_rgb, * ibuf_ir;
  int full, buf_index;
  
  if(SETUP_STREAMS & SETUP_STREAM_RGB) {
    ibuf_rgb = malloc(SETUP_IMAGE_SIZE_RGB);
  }
  
  if(SETUP_STREAMS & SETUP_STREAM_IR) {
    ibuf_ir = malloc(SETUP_IMAGE_SIZE_IR);
  }
  
  while(1) {
    tv.tv_sec = 0;
    tv.tv_usec = SETUP_POLL_DELAY;
    select(0, NULL, NULL, NULL, &tv);
    
    if(!sem_trywait(&input->full)) {
      pthread_mutex_lock(&input->lock);
      sem_getvalue(&input->full, &full);
      buf_index = (unsigned int)((input->serial - full) % SETUP_BUFFER_LENGTH_C2S);
#ifdef DEBUG
      printf("server consuming new frame (serial: %llu, buffer: %d)\n", input->serial, buf_index);
#endif
      // Copy incoming frames to local buffers
      if(SETUP_STREAMS & SETUP_STREAM_RGB) {
	memcpy(ibuf_rgb, input->rgb[buf_index].data, input->rgb[buf_index].size);
      }
      if(SETUP_STREAMS & SETUP_STREAM_IR) {
	memcpy(ibuf_ir, input->ir[buf_index].data, input->ir[buf_index].size);
      }
      
      sem_post(&input->empty);
      pthread_mutex_unlock(&input->lock);
    }
  }

  
  return NULL;
}
