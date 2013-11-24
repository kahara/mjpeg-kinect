#ifndef INTERTHREAD_H
#define INTERTHREAD_H

#include <pthread.h>
#include <inttypes.h>
#include <stddef.h>

struct channel {
  pthread_mutex_t lock; // lock this when doing anything with this structure
  
  uint8_t * buffer_rgb[2]; // double buffer for rgb images
  size_t framesize_rgb; // size of one rgb frame
  
  uint8_t * buffer_ir[2];  // ..same for ir images
  size_t framesize_ir;
  
  int8_t index_rgb; // current "good to read" half of rgb buffer, or -1 if not ready
  int8_t index_ir;  // ..same for ir buffer
  
  
};

struct thread_arg {
  struct channel * input;
  struct channel * output;
};

extern struct channel init_channel(int size_rgb, int size_ir);
extern void uninit_channel(struct channel ch);

#endif // INTERTHREAD_H
