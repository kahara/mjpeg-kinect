#include <stdio.h>
#include "preprocessor.h"
#include "interthread.h"

void * preprocessor(void * args)
{
  struct thread_arg * ta = (struct thread_arg *)args;
  struct channel * input = ta->input, * output = ta->output;
  struct timeval tv;
  
  while(1) {
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    select(0, NULL, NULL, NULL, &tv);
    
    if(!sem_trywait(&input->full)) {
      
      pthread_mutex_lock(&output->lock);
      printf("preprocessor consuming new frame\n");
      sem_post(&input->empty);
      pthread_mutex_unlock(&output->lock);
    }
  }
  
  return NULL;
}
