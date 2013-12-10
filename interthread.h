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
  pthread_mutex_t lock;
  sem_t empty;
  sem_t full;
  
  uint64_t serial;
  
  struct frame * rgb; // buffer for rgb images
  struct frame * ir;  // ..same for ir images
};

struct thread_arg {
  struct channel * input;
  struct channel * output;
};

extern struct channel init_channel(size_t buflen, size_t size_rgb, size_t size_ir);
extern void uninit_channel(struct channel ch);

extern void request_grab(void);
extern time_t last_grab_request;

#endif // INTERTHREAD_H
