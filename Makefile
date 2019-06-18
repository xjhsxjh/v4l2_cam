CC=gcc
CFLAGS=-g -Wall
LDFLAGS=

target=v4l2_cam
objs=$(patsubst %.c, %.o, $(wildcard *.c))

all:$(target)

$(target):$(objs)
	$(CC) $^ -o $@ $(LDFLAGS)

.c.o:
	$(CC) -c $< $(CFLASG)

.PHONY:
	clean

clean:
	rm *.o $(target) -rf

