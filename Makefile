BIN = ymacid
SRC = $(wildcard *.c)
HEADERS = $(wildcard *.h)
CFLAGS = -Wall

$(BIN): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(SRC)

run: $(BIN)
	./$(BIN)

clean:
	-rm -f $(BIN)
