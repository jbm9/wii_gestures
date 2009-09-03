CC = gcc
CFLAGS = -g -Wall -Werror -std=c99 -I../include
LDFLAGS = -lm

all: stamp gesture.o  gesturemodel.o  hmm.o  observation.o  quantizer.o  util.o

stamp:
	touch stamp

clean:
	rm -f *.o stamp

anyway: clean all
