#ifndef INTERTHREAD_H
#define INTERTHREAD_H

#include <pthread.h>
#include <inttypes.h>
#include <stddef.h>
#include <semaphore.h>

struct frame {
  size_t size;
  uint8_t * data;
};

struct channel {
  sem_t empty;
  sem_t full;
  
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
