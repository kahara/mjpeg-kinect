#include <stdio.h>
#include <pthread.h>
#include "settings.h"
#include "grabber.h"
#include "preprocessor.h"
#include "compressor.h"
#include "server.h"

int main(int argc, char **argv)
{
  pthread_t grabber_thread, preprocessor_thread, compressor_thread, server_thread;
  
  pthread_create(&grabber_thread, NULL, &grabber, NULL);
  pthread_create(&preprocessor_thread, NULL, &grabber, NULL);
  pthread_create(&compressor_thread, NULL, &grabber, NULL);
  pthread_create(&server_thread, NULL, &grabber, NULL);
  
  return 0;
}
