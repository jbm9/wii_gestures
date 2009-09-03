JAVA = $(patsubst %.java, %.class, $(shell find -name '*.java' -print))

all: $(JAVA)

%.class: %.java
	javac $<

clean:
	rm -f $(JAVA)

anyway: clean all

.PHONY: clean all anyway
