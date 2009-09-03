#!/bin/sh

set -e
set -x

for i in quantizer_data/*.dat 
do
	java -classpath ../javatest QuantizerTest < $i > tmp.1
	./quantizer_test                          < $i > tmp.2
	diff -u tmp.1 tmp.2
	rm   -f tmp.1 tmp.2
	echo
done
