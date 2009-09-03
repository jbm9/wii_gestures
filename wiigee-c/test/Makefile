CC = gcc
CFLAGS = -g -Wall -Werror -std=c99 -I../include
LDFLAGS = -lm

all: stamp \
     gesturemodel_test.o  hmm_test.o  quantizer_test.o

stamp:
	touch stamp

clean:
	rm -f *.o stamp

anyway: clean all
