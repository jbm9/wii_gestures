all: lib

lib:
	$(MAKE) -C lib

javatest:
	$(MAKE) -C javatest

test: lib javatest
	$(MAKE) -C test

clean:
	$(MAKE) -C lib      clean
	$(MAKE) -C test     clean
	$(MAKE) -C javatest clean

anyway: clean all

.PHONY: all lib javatest test clean anyway
