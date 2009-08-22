CC = gcc
CFLAGS = -g -Wall -Werror -std=c99
LDFLAGS = $(CFLAGS) -lm

all: quantizer

quantizer: util.o quantizer.o main.o

clean:
	rm -f quantizer *.o

anyway: clean all
