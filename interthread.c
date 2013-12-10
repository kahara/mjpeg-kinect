#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include "interthread.h"

struct channel init_channel(size_t buflen, size_t size_rgb, size_t size_ir)
{
  struct channel ch;
  int i;
  
  pthread_mutex_init(&ch.lock, NULL);
  sem_init(&ch.empty, 0, buflen);
  sem_init(&ch.full, 0, 0);
  
  ch.serial = -1;
  
  ch.rgb = buflen ? malloc(sizeof(struct frame) * buflen) : NULL;
  ch.ir = buflen ? malloc(sizeof(struct frame) * buflen) : NULL;
  
  for(i = 0; i < buflen; i++) {
    if(size_rgb) {
      ch.rgb[i].size = 0;
      ch.rgb[i].data = malloc(size_rgb);      
    }
    
    if(size_ir) {
      ch.ir[i].size = 0;
      ch.ir[i].data = malloc(size_ir);
    }
  }
  
  return ch;
}

void uninit_channel(struct channel ch)
{
  // ...
}

time_t get_or_set_grab_request(int update)
{
  static time_t last_grab_request;
  
  if(update)
    time(&last_grab_request); // this should be atomic, rigth?
  
  return last_grab_request;
}
