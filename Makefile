COMMIT ?= $(shell bash -c 'read -p "Commit message: " msg; echo $$msg')

CC=gcc

SRC=src
OBJ=obj
BIN=bin
LIBDIR=lib

CFLAGS=-Wall -g -lpthread 

STATICLIB = lib/lib.a 

all: clean $(STATICLIB) $(BIN)/farm $(BIN)/master $(BIN)/collector $(BIN)/generafile   

obj/%.o: src/%.c $(STATICLIB)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN)/farm: $(OBJ)/farm.o $(STATICLIB)
	$(CC) $(CFLAGS) $(OBJ)/farm.o -o $@ 

$(BIN)/master: $(OBJ)/master.o $(STATICLIB)
	$(CC) $(CFLAGS) $(OBJ)/master.o  -o $@ $(STATICLIB)


$(BIN)/collector: $(OBJ)/collector.o $(STATICLIB) 
	$(CC) $(CFLAGS) $(OBJ)/collector.o -o $@ $(STATICLIB)

$(BIN)/generafile: $(OBJ)/generafile.o
	$(CC) $(CFLAGS) $(OBJ)/generafile.o -o $@ 

clean:
	- rm -r bin/* obj/* tmp/* lib/obj/* lib/*.a testdir/* 
	- rm file* 
	- rm -r testdir
	- rm collector expected.txt farm generafile master 

push:
	git add .
	git commit -m "$(COMMIT)"
	git push origin

$(STATICLIB) : $(LIBDIR)/$(SRC)/*
	@for f in $^; do $(CC) $(CFLAGS) -c $${f} ;  done
	@for f in $(shell ls ${LIBDIR}/${SRC}); do mv $${f%%.*}.o $(LIBDIR)/$(OBJ) ; done
	-ar -rvs $(STATICLIB) $(LIBDIR)/$(OBJ)/*

test: all
	- cp bin/* .
	- chmod +x test.sh && ./test.sh


.PHONY: all clean push valgrind test