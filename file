CC = gcc
CFLAGS = -Wall -Iinclude
LDFLAGS = -lreadline

TARGET = shellforge

SRC = src/main.c src/history.c src/lexer.c src/token.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)
