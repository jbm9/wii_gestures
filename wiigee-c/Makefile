CC = gcc
CFLAGS = -g -Wall -Werror -std=c99
LDFLAGS = $(CFLAGS) -lm

all: quantizer

quantizer: util.o quantizer.o main.o

build_test:
	make -C javatest

test: all build_test   $(shell ls test_data/*.dat | sed 's/\.dat/.runtest/')

%.runtest: %.dat
	@echo
	@echo == Testing $<
	java -classpath javatest/ TestHarness < $< > tmp.1
	./quantizer < $< > tmp.2
	diff -u tmp.1 tmp.2
	rm -f tmp.1 tmp.2
	@echo

clean: clean_test
	rm -f quantizer *.o

clean_test:
	make -C javatest clean

anyway: clean all
