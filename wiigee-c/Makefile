all: lib javatest test

lib/stamp:
lib: lib/stamp
	make -C lib

test/stamp:
test: lib javatest test/stamp
	make -C test

javatest/stamp:
javatest: javatest/stamp
	make -C javatest

clean:
	make -C lib clean
	make -C test clean
	make -C javatest clean

anyway: clean all
