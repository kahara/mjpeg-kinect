CC = gcc
CFLAGS = -g -Wall -pthread -DDEBUG
LIBS = -lrt -ljpeg

mjpeg-kinect: main.o grabber.o preprocessor.o compressor.o server.o interthread.o
	$(CC) $(CFLAGS) $^ -o mjpeg-kinect $(LIBS)

main.o: main.c settings.h grabber.h
	$(CC) $(CFLAGS) -c main.c

grabber.o: grabber.c grabber.h settings.h 
	$(CC) $(CFLAGS) -c grabber.c

preprocessor.o: preprocessor.c preprocessor.h settings.h 
	$(CC) $(CFLAGS) -c preprocessor.c

compressor.o: compressor.c compressor.h settings.h 
	$(CC) $(CFLAGS) -c compressor.c

server.o: server.c server.h settings.h 
	$(CC) $(CFLAGS) -c server.c

interthread.o: interthread.c interthread.h settings.h
	$(CC) $(CFLAGS) -c interthread.c

.PHONY: clean
clean:
	rm *.o
