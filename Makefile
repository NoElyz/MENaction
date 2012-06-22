#Makefile for tutorial

BUILTIN_CFLAGS=-std=c99
CC?=gcc
LD?=gcc

# Pkg-Config
BUILTIN_CFLAGS="$(BUILTIN_CFLAGS) $(shell pkg-config libavcodec --cflags)"
BUILTIN_CFLAGS="$(BUILTIN_CFLAGS) $(shell pkg-config libavformat --cflags)"
BUILTIN_CFLAGS="$(BUILTIN_CFLAGS) $(shell pkg-config libavutil --cflags)"
BUILTIN_CFLAGS="$(BUILTIN_CFLAGS) $(shell pkg-config libswscale --cflags)"

BUILTIN_LDFLAGS="$(BUILTIN_LDFLAGS) $(shell pkg-config libavcodec --libs)"
BUILTIN_LDFLAGS="$(BUILTIN_LDFLAGS) $(shell pkg-config libavformat --libs)"
BUILTIN_LDFLAGS="$(BUILTIN_LDFLAGS) $(shell pkg-config libavutil --libs)"
BUILTIN_LDFLAGS="$(BUILTIN_LDFLAGS) $(shell pkg-config libswscale --libs)"


OBJS=f2s.o
TARGET=MENaction

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "LD	$@"
	@$(LD) -o $(TARGET) $(OBJS) $(BUILTIN_LDFLAGS) $(LDFLAGS)

.c.o:
	@echo "CC	$@"
	@$(CC) $(BUILTIN_CFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	@echo "RM	$(OBJS)"
	@rm -f $(OBJS)

distclean: clean
	@echo "RM	$(TARGET)“
	@rm -f $(TARGET)