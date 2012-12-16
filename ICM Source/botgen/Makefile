# Project: botgen

CC   = g++
OBJ  = botgen.o $(RES)
BIN  = botgen
CFLAGS = 

.PHONY: all all-before all-after clean clean-custom

all: all-before botgen all-after


clean: clean-custom
	rm -f $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o "botgen" $(CFLAGS)

botgen.o: botgen.cpp
	$(CC) -c botgen.cpp -o botgen.o $(CFLAGS)
