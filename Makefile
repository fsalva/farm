COMMIT ?= $(shell bash -c 'read -p "Commit message: " msg; echo $$msg')

CC=gcc

SRC=src
OBJ=obj
BIN=bin

LIBDIR=lib
CFLAGS=-Wall -lpthread 


all: $(BIN)/farm $(BIN)/master $(BIN)/collector $(BIN)/generafile   

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN)/farm: $(OBJ)/farm.o
	$(CC) $(CFLAGS) $(OBJ)/farm.o -o $@ 


$(BIN)/master: $(OBJ)/master.o
	$(CC) $(CFLAGS) $(OBJ)/master.o -o $@ 


$(BIN)/collector: $(OBJ)/collector.o
	$(CC) $(CFLAGS) $(OBJ)/collector.o -o $@ 


$(BIN)/generafile: $(OBJ)/generafile.o
	$(CC) $(CFLAGS) $(OBJ)/generafile.o -o $@ 

clean:
	rm -r bin/* obj/* tmp/*

push:
	git add .
	git commit -m "$(COMMIT)"
	git push origin

.PHONY: all clean push