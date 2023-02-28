COMMIT ?= $(shell bash -c 'read -p "Commit message: " msg; echo $$msg')

CC=gcc

SRC=src
OBJ=obj
BIN=bin

LIBDIR=lib
CFLAGS=-Wall -g -lpthread 

LIBPATH = -l./lib/include/
ARTIPATH = -L./lib/

ARTIFACT = lib/lib.a 

all: clean $(ARTIFACT) $(BIN)/farm $(BIN)/master $(BIN)/collector $(BIN)/generafile   


obj/%.o: src/%.c $(ARTIFACT)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN)/farm: $(OBJ)/farm.o $(ARTIFACT)
	$(CC) $(CFLAGS) $(OBJ)/farm.o -o $@ 


$(BIN)/master: $(OBJ)/master.o $(ARTIFACT)
	$(CC) $(CFLAGS) $(OBJ)/master.o  -o $@ $(ARTIFACT)


$(BIN)/collector: $(OBJ)/collector.o $(ARTIFACT) 
	$(CC) $(CFLAGS) $(OBJ)/collector.o -o $@ 


$(BIN)/generafile: $(OBJ)/generafile.o
	$(CC) $(CFLAGS) $(OBJ)/generafile.o -o $@ 

clean:
	- rm -r bin/* obj/* tmp/* lib/obj/* lib/*.a

push:
	git add .
	git commit -m "$(COMMIT)"
	git push origin

$(ARTIFACT) : $(LIBDIR)/$(SRC)/*
	@for f in $^; do $(CC) $(CFLAGS) -c $${f} ;  done
	@for f in $(shell ls ${LIBDIR}/${SRC}); do mv $${f%%.*}.o $(LIBDIR)/$(OBJ) ; done
	-ar -rvs $(ARTIFACT) $(LIBDIR)/$(OBJ)/*


valgrind: 
	- make clean 
	- make all
	valgrind --leak-check=full --trace-children=yes --show-leak-kinds=all --track-origins=yes bin/farm -d filetest > valgr.txt 2>&1

.PHONY: all clean push valgrind