CC = gcc

CFLAGS = -Wall

TARGET = test

all: $(TARGET)

$(TARGET): fread_ex.c
	$(CC) $(CFLAGS) -o $(TARGET) fread_ex.c

clean:
	rm -f $(TARGET)
