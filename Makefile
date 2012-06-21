#Makefile for tutorial

BUILTIN_CFLAGS=-std=c99
CC=gcc
LD=gcc
CFLAGS=`pkg-config libavformat --cflags` `pkg-config libavutil --cflags` `pkg-config libavcodec --cflags` `pkg-config libswscale --cflags`
LIBS=`pkg-config libavformat --libs` `pkg-config libavcodec --libs` `pkg-config libswscale --libs` `pkg-config libavutil --libs`

OBJS=f2s.o
TARGET=MENaction

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LIBS)

.c.o:
	echo "CC	$@"
	$(CC) $(BUILTIN_CFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	@echo "RM	$(TARGET) $(OBJS)"
	@rm -f $(TARGET) $(OBJS)
