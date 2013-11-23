CFLAGS = -g -Wall -pthread
LIBS = -lrt
SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

mjpeg-kinect: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS)  -c  $<

.PHONY: clean
clean:
	rm *.o
