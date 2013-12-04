#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#include "settings.h"
#include "compressor.h"
#include "interthread.h"

void compress_rgb(uint8_t * in, uint8_t * out, int width, int height);
void compress_ir(uint8_t * in, uint8_t * out, int width, int height);

void * compressor(void * args)
{
  struct thread_arg * ta = (struct thread_arg *)args;
  struct channel * input = ta->input, * output = ta->output;
  struct timeval tv;
  uint8_t * ibuf_rgb, * obuf_rgb, * ibuf_ir, * obuf_ir;
  int full, buf_index;
  
  if(SETUP_STREAMS & SETUP_STREAM_RGB) {
    ibuf_rgb = malloc(SETUP_IMAGE_SIZE_RGB);
    obuf_rgb = malloc(SETUP_IMAGE_SIZE_RGB);
  }
  
  if(SETUP_STREAMS & SETUP_STREAM_IR) {
    ibuf_ir = malloc(SETUP_IMAGE_SIZE_IR);
    obuf_ir = malloc(SETUP_IMAGE_SIZE_IR);
  }
  
  while(1) {
    tv.tv_sec = 0;
    tv.tv_usec = SETUP_POLL_DELAY;
    select(0, NULL, NULL, NULL, &tv);
    
    if(!sem_trywait(&input->full)) {
      pthread_mutex_lock(&input->lock);
      sem_getvalue(&input->full, &full);
      buf_index = (unsigned int)((input->serial - full) % SETUP_BUFFER_LENGTH_P2C);
#ifdef DEBUG
      printf("compressor consuming new frame (serial: %llu, buffer: %d)\n", input->serial, buf_index);
#endif
      // Copy incoming frames to local buffers
      if(SETUP_STREAMS & SETUP_STREAM_RGB) {
	memcpy(ibuf_rgb, input->rgb[buf_index].data, SETUP_IMAGE_SIZE_RGB);
      }
      if(SETUP_STREAMS & SETUP_STREAM_IR) {
	memcpy(ibuf_ir, input->ir[buf_index].data, SETUP_IMAGE_SIZE_IR);
      }
      
      sem_post(&input->empty);
      pthread_mutex_unlock(&input->lock);
      
      // compress frames
      if(SETUP_STREAMS & SETUP_STREAM_RGB) {
	compress_rgb(ibuf_rgb, obuf_rgb, SETUP_IMAGE_WIDTH_RGB, SETUP_IMAGE_HEIGHT_RGB);
      }
      if(SETUP_STREAMS & SETUP_STREAM_IR) {
	compress_ir(ibuf_ir, obuf_ir, SETUP_IMAGE_WIDTH_IR, SETUP_IMAGE_HEIGHT_IR);
      }
      
      if(sem_trywait(&output->empty)) {
#ifdef DEBUG
	printf("compressor dropping frame\n");
#endif
      } else {
	pthread_mutex_lock(&output->lock);
#ifdef DEBUG
        printf("compressor producing new frame\n");
#endif	
	output->serial++;
	
	if(SETUP_STREAMS & SETUP_STREAM_RGB) {
	  output->rgb[output->serial % SETUP_BUFFER_LENGTH_C2S].size = SETUP_IMAGE_SIZE_RGB;
	  memcpy(output->rgb[output->serial % SETUP_BUFFER_LENGTH_C2S].data, obuf_rgb, SETUP_IMAGE_SIZE_RGB);
	}
	
	if(SETUP_STREAMS & SETUP_STREAM_IR) {
	  output->ir[output->serial % SETUP_BUFFER_LENGTH_C2S].size = SETUP_IMAGE_SIZE_IR;
	  memcpy(output->ir[output->serial % SETUP_BUFFER_LENGTH_C2S].data, obuf_ir, SETUP_IMAGE_SIZE_IR);
	}
	
        sem_post(&output->full);
	pthread_mutex_unlock(&output->lock);
      }
    }
  }
  
  return NULL;
}

void compress_rgb(uint8_t * in, uint8_t * out, int width, int height)
{
  
}

void compress_ir(uint8_t * in, uint8_t * out, int width, int height)
{
  
}
