CC = gcc

CFLAGS = -Wall

TARGET = test

all: $(TARGET)

$(TARGET): setuid_ex.c
	$(CC) $(CFLAGS) -o $(TARGET) setuid_ex.c

clean:
	rm -f $(TARGET)
