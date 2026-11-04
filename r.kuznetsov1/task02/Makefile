CC      = gcc
CFLAGS  = -Wall -Wextra -O2
TARGET  = ex_time
SRC     = ex_time.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
