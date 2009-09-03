CC = gcc
CFLAGS = -g -Wall -Werror -std=c99 -I../include
LDFLAGS = -lm

all: libwiigee.so stamp

OBJS = $(patsubst %.c, %.o, $(shell ls *.c))

libwiigee.so: $(OBJS)
	ar rcus libwiigee.a $(OBJS)
	ld -o libwiigee.so $(OBJS) -shared -soname libwiigee.so

stamp:
	touch stamp

clean:
	rm -f *.o *.a *.so stamp

anyway: clean all
