CC = gcc
CFLAGS = -g -Wall -Werror -std=c99
#LDFLAGS = $(CFLAGS) -lm
LDFLAGS = -lm

all: lib test

lib/stamp:
lib: lib/stamp
	make -C lib

test/stamp:
test: test/stamp
	make -C test

clean:
	make -C lib clean
	make -C test clean

anyway: clean all

#
#all:           hmm quantizer gesturemodel
#quantizer:     util.o observation.o gesture.o quantizer.o hmm.o    quantizer_test.o
#hmm:           util.o observation.o gesture.o quantizer.o hmm.o    hmm_test.o
#gesturemodel:  util.o observation.o gesture.o quantizer.o hmm.o    gesturemodel_test.o
#
#test: test_quantizer
#
#test_quantizer: quantizer build_quantizer_java_test $(shell ls test_data/*.dat | sed 's/\.dat/.runtest/')
#
#build_quantizer_java_test:
#	make -C javatest
#
#%.runtest: %.dat
#	@echo
#	@echo == Testing $<
#	java -classpath javatest/ TestHarness < $< > tmp.1
#	./quantizer < $< > tmp.2
#	diff -u tmp.1 tmp.2
#	rm -f tmp.1 tmp.2
#	@echo
#
#
#clean: clean_test
#	rm -f quantizer hmm gesturemodel *.o
#
#clean_test:
#	make -C javatest clean
#
#anyway: clean all
