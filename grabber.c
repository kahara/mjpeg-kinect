#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include "settings.h"
#include "grabber.h"
#include "interthread.h"

void * grabber(void * args)
{
  struct thread_arg * ta = (struct thread_arg *)args;
  struct channel * input = ta->input, * output = ta->output;
  
  while(1) {
    
    pthread_mutex_lock(&input->lock);
    pthread_cond_wait(&input->new_frame, &input->lock);
    pthread_mutex_unlock(&input->lock);
    printf("grab new frame\n");
  }
  
  return NULL;
}
