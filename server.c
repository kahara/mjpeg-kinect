#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/select.h>
#include "settings.h"
#include "server.h"
#include "interthread.h"

void * serve(void * c);

struct channel * frames;

void * server(void * args)
{
  struct thread_arg * ta = (struct thread_arg *)args;
  
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
void * serve(void * c)
{
  int conn = (int)c; // passing a socket like this is a form of abuse but it works
  
#define BLOCK_SIZE 1024
#define REQUEST_MAX 65536
  char block[BLOCK_SIZE];
  char * buffer_in;
  size_t buffer_size = 0;
  
  fd_set fds;
  struct timeval tv;
  int len;
  
  FD_ZERO(&fds);
  FD_SET(conn, &fds);
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  if(!select(conn + 1, &fds, NULL, NULL, &tv)) { // client didn't send anything; terminate
    close(conn);
    return NULL;
  }
  
  while(1) {
    len = read(conn, block, BLOCK_SIZE);
    buffer_size += len;
    
    if(buffer_size > REQUEST_MAX) { // request too long; terminate
      close(conn);
      return NULL;
    }
    
    buffer_in = realloc(buffer_in, buffer_size + 1);
    
    printf("%p %p (%d)\n", buffer_in, buffer_in + buffer_size, len);
    memcpy(buffer_in + buffer_size - len, block, len);
    buffer_in[buffer_size] = 0;
    
    FD_ZERO(&fds);
    FD_SET(conn, &fds);
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if(!select(conn + 1, &fds, NULL, NULL, &tv))
      break;
  }
  
  request_grab();
  
  printf("%d: %s\n", buffer_size, buffer_in);
  
  close(conn);
  return NULL;
}
