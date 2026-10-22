CC = gcc

CFLAGS = -Wall

TARGET = test

all: $(TARGET)
$(TARGET): getopt_ex.c
	$(CC) $(CFLAGS) -o $(TARGET) getopt_ex.c

clean:
	rm -f $(TARGET)
