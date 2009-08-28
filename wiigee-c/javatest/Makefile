all:	TestHarness.class \
	logic/Gesture.class  \
	logic/Quantizer.class 

%.class: %.java
	javac $<

clean:
	find -name '*.class' | xargs rm -f

anyway: clean all
