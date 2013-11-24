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
  struct timeval tv;
  
  printf("%p\n%p\n", input, output);
  
  while(1) {
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    select(0, NULL, NULL, NULL, &tv);
    printf("grabber\n");
  }
  
  return NULL;
}
