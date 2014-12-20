# Project: objn64
# Makefile created by Dev-C++ 4.9.9.2
# Adapted to *nix by spinout

CPP  = g++
CC   = gcc
OBJ  = main.o objload.o n64out.o
BIN  = objn64
CXXFLAGS = -O2
CFLAGS = -O3 -U_FORTIFY_SOURCE
RM = rm -f
LFLAGS = -lm

default: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(LFLAGS)

clean:
	$(RM) $(OBJ) $(BIN)

