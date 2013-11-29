#ifndef INTERTHREAD_H
#define INTERTHREAD_H

#include <pthread.h>
#include <inttypes.h>
#include <stddef.h>

struct frame {
  size_t size;
  uint8_t * data;
};

struct channel {
  pthread_mutex_t lock; // producer signals consumer with cv that a new frame is available
  pthread_cond_t new_frame;
  
  struct frame * rgb; // buffer for rgb images
  struct frame * ir;  // ..same for ir images
};

struct thread_arg {
  struct channel * input;
  struct channel * output;
};

extern struct channel init_channel(size_t buflen, size_t size_rgb, size_t size_ir);
extern void uninit_channel(struct channel ch);

#endif // INTERTHREAD_H
