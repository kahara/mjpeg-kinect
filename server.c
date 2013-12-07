#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "settings.h"
#include "server.h"
#include "interthread.h"

void * serve(void * args);

struct arg {
  // xxx ...
  struct channel * input;
};

void * server(void * args)
{
  struct thread_arg * ta = (struct thread_arg *)args;
  struct channel * input = ta->input;
  struct timeval tv;
  
  int sock, connection;
  struct sockaddr_in addr;
  pthread_t threads[SETUP_SERVER_THREADS] = { 0 };
  
  // xxx add some error checking logic here
  sock = socket(AF_INET, SOCK_STREAM, 0);
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(SETUP_SERVER_ADDRESS);
  addr.sin_port = htons(SETUP_SERVER_PORT);
  bind(sock, (struct sockaddr *)&addr, sizeof(addr));
  listen(sock, 8);
  
  while(1) {
    connection = accept(sock, (struct sockaddr *)NULL, NULL);
    printf("CONNECTION\n");
    close(connection);
    
    //pthread_create(&threads[0], NULL, &serve, input);
    
    //tv.tv_sec = 0;
    //tv.tv_usec = SETUP_POLL_DELAY;
    //select(0, NULL, NULL, NULL, &tv);
  }
  
  
  return NULL;
}

// serving connections with dedicated threads may not scale too well
void * serve(void * args)
{
  struct channel * ch = (struct channel *)args;
  
  return NULL;
}
