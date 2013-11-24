#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include "interthread.h"

struct channel init_channel(int size_rgb, int size_ir)
{
  struct channel ch;
  
  pthread_mutex_init(&ch.lock, NULL);
  
  ch.buffer_rgb[0] = malloc(size_rgb);
  ch.buffer_rgb[1] = malloc(size_rgb);
  ch.framesize_rgb = size_rgb;
  
  ch.buffer_ir[0] = malloc(size_ir);
  ch.buffer_ir[1] = malloc(size_ir);
  ch.framesize_ir = size_ir;
  
  ch.index_rgb = -1;
  ch.index_ir = -1;
  
  return ch;
}

void uninit_channel(struct channel ch)
{
  free(ch.buffer_rgb[0]);
  free(ch.buffer_rgb[1]);
  
  free(ch.buffer_ir[0]);
  free(ch.buffer_ir[1]);
}
