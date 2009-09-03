CC = gcc
CFLAGS = -g -Wall -Werror -std=c99 -I../include
LDFLAGS = -lm

all: stamp libwiigee.a

libwiigee.a:  gesture.o  gesturemodel.o  hmm.o  observation.o  quantizer.o  util.o
	ar rcu libwiigee.a *.o
	ranlib libwiigee.a
	ld -o libwiigee.so *.o -shared -soname libwiigee.so

stamp:
	touch stamp

clean:
	rm -f *.a *.so *.o stamp

anyway: clean all
