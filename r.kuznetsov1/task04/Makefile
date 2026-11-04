CC      = gcc
CFLAGS  = -Wall -Wextra -O2
TARGET  = list
SRC     = list.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
