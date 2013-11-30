#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/select.h>
#include <inttypes.h>
#include <stdlib.h>
#include "settings.h"
#include "grabber.h"
#include "preprocessor.h"
#include "compressor.h"
#include "server.h"
#include "interthread.h"

struct channel ch_g2p, ch_p2c, ch_c2s;

int main(int argc, char **argv)
{
  struct thread_arg arg_grabber = { NULL, &ch_g2p }, arg_preprocessor = { &ch_g2p, &ch_p2c }, arg_compressor = { &ch_p2c, &ch_c2s }, arg_server = { &ch_c2s, NULL };
  pthread_t grabber_thread, preprocessor_thread, compressor_thread, server_thread;
  struct timeval tv;
  
  // Set up inter-thread communications "channels"
  // Compressor to Server makes the assumption that a JPEG compressed image will never be bigger than the source bitmap
  ch_g2p = init_channel(SETUP_BUFFER_LENGTH_G2P, \
			(SETUP_STREAMS & SETUP_STREAM_RGB) ? SETUP_IMAGE_SIZE_RAW_RGB : 0, \
			(SETUP_STREAMS & SETUP_STREAM_IR) ? SETUP_IMAGE_SIZE_RAW_IR : 0);     // Grabber      to Preprocessor

  ch_p2c = init_channel(SETUP_BUFFER_LENGTH_P2C, \
			(SETUP_STREAMS & SETUP_STREAM_RGB) ? SETUP_IMAGE_SIZE_RGB : 0, \
			(SETUP_STREAMS & SETUP_STREAM_IR) ? SETUP_IMAGE_SIZE_IR : 0);         // Preprocessor to Compressor

  ch_c2s = init_channel(SETUP_BUFFER_LENGTH_C2S, \
			(SETUP_STREAMS & SETUP_STREAM_RGB) ? SETUP_IMAGE_SIZE_RGB : 0, \
			(SETUP_STREAMS & SETUP_STREAM_IR) ? SETUP_IMAGE_SIZE_IR : 0);         // Compressor   to Server
  
  // Start the "subsystems"
  pthread_create(&grabber_thread, NULL, &grabber, &arg_grabber);
  pthread_create(&preprocessor_thread, NULL, &preprocessor, &arg_preprocessor);
  pthread_create(&compressor_thread, NULL, &compressor, &arg_compressor);
  pthread_create(&server_thread, NULL, &server, &arg_server);
  
  // Do nothing; use select(2) here because sleep/usleep etc. rely on SIGALRM (as we do, see above)
  while(1) {
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    select(0, NULL, NULL, NULL, &tv);
  }
  
  return 0;
}
