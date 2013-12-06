#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "settings.h"
#include "server.h"
#include "interthread.h"

void * server(void * args)
{
  struct thread_arg * ta = (struct thread_arg *)args;
  struct channel * input = ta->input;
  struct timeval tv;
  
  while(1) {
    tv.tv_sec = 0;
    tv.tv_usec = SETUP_POLL_DELAY;
    select(0, NULL, NULL, NULL, &tv);    
  }
  
  return NULL;
}
