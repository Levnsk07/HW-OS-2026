CC      = gcc
CFLAGS  = -Wall -Wextra -O2
TARGET  = getopt_ex
SRC     = getopt_ex.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
