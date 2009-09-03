OBJS = $(patsubst %.java, %.class, $(shell find -name '*.java' -print))

all:	$(OBJS) stamp

%.class: %.java
	javac $<

stamp:
	touch stamp

clean:
	find -name '*.class' -o -name stamp | xargs rm -f

anyway: clean all
