EXECUTABLE = dump_infoframes
OBJECTS = dump_infoframes.o

CFLAGS = -g -Wall -Wextra -pedantic -Wno-pointer-arith -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE 

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)

clean:
	-rm -f $(EXECUTABLE) $(OBJECTS)

.PHONY: all clean
