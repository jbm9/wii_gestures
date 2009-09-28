CC = gcc
CFLAGS = -g -Wall -Werror -std=c99 -I../include
LDFLAGS = -lm

OBJS = $(patsubst %.c, %.o, $(shell ls *.c))

all: libwiigee.so

libwiigee.so: $(OBJS)
	ar rcus libwiigee.a $(OBJS)
	ld -o libwiigee.so $(OBJS) -shared -soname libwiigee.so

clean:
	rm -f $(OBJS) libwiigee.a libwiigee.so

anyway: clean all

.PHONY: clean all anyway
