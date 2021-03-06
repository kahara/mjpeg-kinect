CC = gcc
CFLAGS = -g -O0 -Wall -pthread -DDEBUG
#CFLAGS = -O3 -Wall -pthread
STATIC = -static
#STATIC = 
LIBS = -lrt -ljpeg

mjpeg-kinect: main.o grabber.o preprocessor.o compressor.o server.o interthread.o
	$(CC) $(STATIC) $(CFLAGS) $^ -o mjpeg-kinect $(LIBS)

main.o: main.c settings.h grabber.h
	$(CC) $(CFLAGS) -c $<

grabber.o: grabber.c grabber.h settings.h 
	$(CC) $(CFLAGS) -c $<

preprocessor.o: preprocessor.c preprocessor.h settings.h 
	$(CC) $(CFLAGS) -c $<

compressor.o: compressor.c compressor.h settings.h 
	$(CC) $(CFLAGS) -c $<

server.o: server.c server.h settings.h 
	$(CC) $(CFLAGS) -c $<

interthread.o: interthread.c interthread.h settings.h
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm *.o
