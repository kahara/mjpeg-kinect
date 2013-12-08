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

void * serve(void * conn);

struct channel * frames;

void * server(void * args)
{
  struct thread_arg * ta = (struct thread_arg *)args;
  struct timeval tv;
  
  int sock, connection, optval;
  struct sockaddr_in addr, addr_conn;
  socklen_t alen;
  char ipaddress[16];
  pthread_t thread;
  
  frames = ta->input;
  
  // xxx add some error checking logic here
  sock = socket(AF_INET, SOCK_STREAM, 0);
  optval = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(SETUP_SERVER_ADDRESS);
  addr.sin_port = htons(SETUP_SERVER_PORT);
  bind(sock, (struct sockaddr *)&addr, sizeof(addr));
  listen(sock, 8);
  
  while(1) {
    connection = accept(sock, (struct sockaddr *)&addr_conn, &alen);
#ifdef DEBUG
    inet_ntop(AF_INET, &addr_conn.sin_addr.s_addr, ipaddress, alen);
    printf("connection (%d) from %s:%d\n", connection, ipaddress, addr_conn.sin_port);
#endif
    
    pthread_create(&thread, NULL, &serve, (void *)connection);
  }
  
  return NULL;
}

// serving connections with dedicated threads may not scale too well
void * serve(void * conn)
{
  int connection = (int)conn; // a form of abuse but it works
  
  close(connection);
  return NULL;
}
