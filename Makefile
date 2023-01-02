COMMIT ?= $(shell bash -c 'read -p "Commit message: " msg; echo $$msg')

CC=gcc

SRC=src
OBJ=obj/farm.o
BIN=./bin/farm

LIBDIR=lib
CFLAGS=-Wall

all: $(BIN)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

clean:
	rm -r bin/* obj/*

push:
	git add .
	git commit -m "$(COMMIT)"
	git push origin