#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <jpeglib.h>
#include "settings.h"
#include "compressor.h"
#include "interthread.h"

size_t compress_rgb(uint8_t * in, uint8_t * out, int width, int height);
size_t compress_ir(uint8_t * in, uint8_t * out, int width, int height);

void * compressor(void * args)
{
  struct thread_arg * ta = (struct thread_arg *)args;
  struct channel * input = ta->input, * output = ta->output;
  struct timeval tv;
  uint8_t * ibuf_rgb, * obuf_rgb, * ibuf_ir, * obuf_ir;
  int full, buf_index;
  size_t size_rgb, size_ir;
#ifdef DEBUG
  char filename[1024];
  int f;
#endif
  
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
	size_rgb = compress_rgb(ibuf_rgb, obuf_rgb, SETUP_IMAGE_WIDTH_RGB, SETUP_IMAGE_HEIGHT_RGB);
      }
      if(SETUP_STREAMS & SETUP_STREAM_IR) {
	size_ir = compress_ir(ibuf_ir, obuf_ir, SETUP_IMAGE_WIDTH_IR, SETUP_IMAGE_HEIGHT_IR);
      }
      
      // pass frame to Server
      // no semaphores are involved and the buffer is "free-running"
      pthread_mutex_lock(&output->lock);
      output->serial++;
      
#ifdef DEBUG
      printf("compressor producing new frame of size %u (rgb), %u (ir)\n", size_rgb, size_ir);
      // xxx check existence of dump directory and create it if it does not exist
      if(size_rgb) {
	sprintf(filename, "./dump/rgb-%05llu.jpg", output->serial);
	f = open(filename, O_CREAT | O_RDWR);
	write(f, obuf_rgb, size_rgb);
	close(f);
      }
      
      if(size_ir) {
	// ...
      }
#endif
      
      if(SETUP_STREAMS & SETUP_STREAM_RGB) {
	output->rgb[output->serial % SETUP_BUFFER_LENGTH_C2S].size = size_rgb;
	memcpy(output->rgb[output->serial % SETUP_BUFFER_LENGTH_C2S].data, obuf_rgb, size_rgb);
      }
      
      if(SETUP_STREAMS & SETUP_STREAM_IR) {
	output->ir[output->serial % SETUP_BUFFER_LENGTH_C2S].size = size_ir;
	memcpy(output->ir[output->serial % SETUP_BUFFER_LENGTH_C2S].data, obuf_ir, size_ir);
      }
      
      pthread_mutex_unlock(&output->lock);
    }
  }
  
  return NULL;
}

// http://stackoverflow.com/a/2296662
size_t compress_rgb(uint8_t * in, uint8_t * out, int width, int height)
{
  struct jpeg_compress_struct cinfo = { 0 };
  struct jpeg_error_mgr jerr;
  JSAMPROW row_ptr[1];
  int row_stride;
  unsigned long int length = 0;
  unsigned char * output = NULL;
  
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_mem_dest(&cinfo, &output, &length);
  
  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;
  
  jpeg_set_defaults(&cinfo);
  jpeg_start_compress(&cinfo, TRUE);
  row_stride = width * 3;
  
  while (cinfo.next_scanline < cinfo.image_height) {
    row_ptr[0] = &out[cinfo.next_scanline * row_stride];
    jpeg_write_scanlines(&cinfo, row_ptr, 1);
  }
  
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  
  memcpy(out, output, length);
  
  return length;
}

size_t compress_ir(uint8_t * in, uint8_t * out, int width, int height)
{
  return 0;
}
