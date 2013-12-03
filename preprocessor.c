#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "settings.h"
#include "preprocessor.h"
#include "interthread.h"

void preprocess_rgb(uint8_t * in, uint8_t * out, int width, int height);
void preprocess_ir(uint8_t * in, uint8_t * out, int width, int height);

void * preprocessor(void * args)
{
  struct thread_arg * ta = (struct thread_arg *)args;
  struct channel * input = ta->input, * output = ta->output;
  struct timeval tv;
  uint8_t * ibuf_rgb, * obuf_rgb, * ibuf_ir, * obuf_ir;
  int full, buf_index;
  
  if(SETUP_STREAMS & SETUP_STREAM_RGB) {
    ibuf_rgb = malloc(SETUP_IMAGE_SIZE_RAW_RGB);
    obuf_rgb = malloc(SETUP_IMAGE_SIZE_RGB);
  }
  
  if(SETUP_STREAMS & SETUP_STREAM_IR) {
    ibuf_ir = malloc(SETUP_IMAGE_SIZE_RAW_IR);
    obuf_ir = malloc(SETUP_IMAGE_SIZE_IR);
  }
  
  while(1) {
    tv.tv_sec = 0;
    tv.tv_usec = SETUP_POLL_DELAY;
    select(0, NULL, NULL, NULL, &tv);
    
    if(!sem_trywait(&input->full)) {
      pthread_mutex_lock(&input->lock);
      sem_getvalue(&input->full, &full);
      buf_index = (unsigned int)((input->serial - full) % SETUP_BUFFER_LENGTH_G2P);
#ifdef DEBUG
      printf("preprocessor consuming new frame (serial: %llu, buffer: %d)\n", input->serial, buf_index);
#endif
      
      sem_post(&input->empty);
      pthread_mutex_unlock(&input->lock);
      
      // Copy incoming frames to local buffers
      if(SETUP_STREAMS & SETUP_STREAM_RGB) {
	memcpy(ibuf_rgb, input->rgb[buf_index].data, SETUP_IMAGE_SIZE_RAW_RGB);
      }
      if(SETUP_STREAMS & SETUP_STREAM_IR) {
	memcpy(ibuf_ir, input->ir[buf_index].data, SETUP_IMAGE_SIZE_RAW_IR);
      }
      
      if(sem_trywait(&output->empty)) {
#ifdef DEBUG
	printf("preprocessor dropping frame\n");
#endif
      } else {
	pthread_mutex_lock(&output->lock);
#ifdef DEBUG
        printf("preprocessor producing new frame\n");
#endif
        sem_post(&output->full);
	pthread_mutex_unlock(&output->lock);
      }
      
      // process frames
      if(SETUP_STREAMS & SETUP_STREAM_RGB) {
	preprocess_rgb(ibuf_rgb, obuf_rgb, SETUP_IMAGE_WIDTH_RGB, SETUP_IMAGE_HEIGHT_RGB);
      }
      if(SETUP_STREAMS & SETUP_STREAM_IR) {
	preprocess_ir(ibuf_ir, obuf_ir, SETUP_IMAGE_WIDTH_IR, SETUP_IMAGE_HEIGHT_IR);
      }
    }
  }
  
  return NULL;
}

// https://github.com/OpenKinect/libfreenect/blob/master/src/cameras.c:convert_bayer_to_rgb() (XXX licensing)
void preprocess_rgb(uint8_t * in, uint8_t * out, int width, int height)
{
  int x,y;
  uint8_t *dst = out;
  uint8_t *prevLine;
  uint8_t *curLine;
  uint8_t *nextLine;
  uint32_t hVals;
  uint32_t vSums;
  uint8_t hSum;
  uint8_t yOdd;
  
  curLine  = in;
  nextLine = curLine + width;
  for (y = 0; y < height; ++y) {
    if ((y > 0) && (y < height-1))
      prevLine = curLine - width;
    else if (y == 0)
      prevLine = nextLine;
    else
      nextLine = prevLine;
    
    hVals  = (*(curLine++) << 8);
    hVals |= (*curLine << 16);
    vSums = ((*(prevLine++) + *(nextLine++)) << 7) & 0xFF00;
    vSums |= ((*prevLine + *nextLine) << 15) & 0xFF0000;
    yOdd = y & 1;
    
    for (x = 0; x < width-1; ++x) {
      hVals |= *(curLine++);
      vSums |= (*(prevLine++) + *(nextLine++)) >> 1;
      hSum = ((uint8_t)(hVals >> 16) + (uint8_t)(hVals)) >> 1;
      
      if (yOdd == 0) {
	if ((x & 1) == 0) {
	  *(dst++) = hSum;
	  *(dst++) = hVals >> 8;
	  *(dst++) = vSums >> 8;
	} else {
	  *(dst++) = hVals >> 8;
	  *(dst++) = (hSum + (uint8_t)(vSums >> 8)) >> 1;
	  *(dst++) = ((uint8_t)(vSums >> 16) + (uint8_t)(vSums)) >> 1;
	}
      } else {
	if ((x & 1) == 0) {
	  *(dst++) = ((uint8_t)(vSums >> 16) + (uint8_t)(vSums)) >> 1;
	  *(dst++) = (hSum + (uint8_t)(vSums >> 8)) >> 1;
	  *(dst++) = hVals >> 8;
	} else {
	  *(dst++) = vSums >> 8;
	  *(dst++) = hVals >> 8;
	  *(dst++) = hSum;
	}
      }
      
      hVals <<= 8;
      vSums <<= 8;
    }
    
    hVals |= (uint8_t)(hVals >> 16);
    vSums |= (uint8_t)(vSums >> 16);
    hSum = (uint8_t)(hVals);
    
    if (yOdd == 0) {
      if ((x & 1) == 0) {
	*(dst++) = hSum;
	*(dst++) = hVals >> 8;
	*(dst++) = vSums >> 8;
      } else {
	*(dst++) = hVals >> 8;
	*(dst++) = (hSum + (uint8_t)(vSums >> 8)) >> 1;
	*(dst++) = vSums;
      }
    } else {
      if ((x & 1) == 0) {
	*(dst++) = vSums;
	*(dst++) = (hSum + (uint8_t)(vSums >> 8)) >> 1;
	*(dst++) = hVals >> 8;
      } else {
	*(dst++) = vSums >> 8;
	*(dst++) = hVals >> 8;
	*(dst++) = hSum;
      }
    }
  }
}

// https://github.com/OpenKinect/libfreenect/blob/master/src/cameras.c:convert_packed_to_8bit() (XXX licensing)
void preprocess_ir(uint8_t * in, uint8_t * out, int width, int height)
{
  int vw = 10, n = width * height;
  uint32_t buffer = 0;
  int bitsIn = 0;
  
  while (n--) {
    while (bitsIn < vw) {
      buffer = (buffer << 8) | *(in++);
      bitsIn += 8;
    }
    bitsIn -= vw;
    *(out++) = buffer >> (bitsIn + vw - 8);
  }
}
