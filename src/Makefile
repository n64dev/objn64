# Project: objn64
#  This Makefile should work on both Windows/MinGW and
#  Linux/GCC targets.

CC   = gcc
OBJ  = main.o objload.o n64out.o
BIN  = objn64
CFLAGS = -O3 -U_FORTIFY_SOURCE
RM = rm -f
LFLAGS = -lm

default: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(LFLAGS)

clean:
	$(RM) $(OBJ) $(BIN)
