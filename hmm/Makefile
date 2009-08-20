# resulting binary name
PROJECT = hmmtest

# required frameworks
FRAMEWORKS = 

LIBS = 
INCLUDEPATH = 

CC = gcc

# happy debug compile for now
CFLAGS = -g -c -Wall
LDFLAGS = 

OBJ_DIR = tmp/build
OUTPUT_DIR = bin
TARGET = $(PROJECT)

SRC_OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(shell find * -name '*.c'))

define build_target
@echo Linking...
@$(CC) -o "$(OUTPUT_DIR)/$(TARGET)" $(SRC_OBJS) $(FRAMEWORKS) $(LDFLAGS) $(LIBS)
endef

define compile_source
@echo Compiling $<
@mkdir -p $(shell dirname $@)
@$(CC) $(CFLAGS) $(INCLUDEPATH) -c "$<" -o "$@"
endef

.PHONY: directories

$(TARGET): directories $(SRC_OBJS)
	$(build_target)

.PHONY: clean

clobber:
	@if [ -d "tmp" ]; then rm -rf tmp; fi
	@if [ -d "build" ]; then rm -rf build; fi

clean:
	@if [ -d "$(OBJ_DIR)" ]; then rm -rf $(OBJ_DIR); fi

directories:
	-@if [ ! -d "$(OUTPUT_DIR)" ]; then mkdir -p "$(OUTPUT_DIR)"; fi
	-@if [ ! -d "$(OBJ_DIR)" ]; then mkdir -p "$(OBJ_DIR)"; fi

$(OBJ_DIR)/%.o: %.c
	$(compile_source)
