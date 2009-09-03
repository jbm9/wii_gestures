CC = gcc
CFLAGS = -g -Wall -Werror -std=c99 -I../include
#LDFLAGS = -lm -L../lib -lwiigee
LDFLAGS = -lm ../lib/*.o

BINARIES = $(patsubst %.c,  %,     $(shell ls *.c))
TESTS    = $(patsubst %.sh, %.run, $(shell ls *.sh))

all: lib $(BINARIES) $(TESTS) stamp
test: all 

%.run: %.sh
	./$<

stamp:
	touch stamp

lib/stamp:
lib: lib/stamp
	make -C ../lib

javatest/stamp:
javatest: javatest/stamp
	make -C javatest

clean:
	rm -f *.o stamp *_test

anyway: clean all
