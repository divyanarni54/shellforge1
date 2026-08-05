CC = gcc
CFLAGS = -Wall -Iinclude

TARGET = shellforge
SRC = src/main.c src/history.c

$(TARGET): $(SRC)
		gcc $(CFLAGS) $(SRC) -lreadline -o $(TARGET)

clean:
		rm -f $(TARGET)
