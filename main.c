#include <stdio.h>
#include <pthread.h>
#include "grabber.h"

int main(int argc, char **argv)
{
  pthread_t grabber_thread;
  
  pthread_create(&grabber_thread, NULL, &grabber, NULL);
  
  
  
  return 0;
}
