CC ?= gcc
CFLAGS ?= -O3 -std=c11
SRC := main.c skiplist.c list.c
OUT := bench

.PHONY: all clean run
all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

run: all
	./$(OUT) 100000 100000 100000 42

clean:
	rm -f $(OUT)
