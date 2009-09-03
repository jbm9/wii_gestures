CC = gcc
CFLAGS = -g -Wall -Werror -std=c99 -I../include
#LDFLAGS = -lm -L../lib -lwiigee
LDFLAGS = -lm ../lib/*.o

BINARIES = $(patsubst %.c,  %,     $(shell ls *.c))
TESTS    = $(patsubst %.sh, %.run, $(shell ls *.sh))

all:  lib $(BINARIES) $(TESTS)
test: all 

%.run: %.sh
	@echo
	@echo == Running test $<
	./$<
	@echo

lib:
	$(MAKE) -C ../lib

javatest:
	$(MAKE) -C javatest

clean:
	rm -f $(BINARIES)

anyway: clean all

.PHONY: clean all anyway test lib javatest
