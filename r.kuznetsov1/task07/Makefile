CC      = gcc
CFLAGS  = -Wall -Wextra -O2
TARGET  = findline_mmap
SRC     = findline_mmap.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
