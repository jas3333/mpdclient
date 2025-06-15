CC = gcc
IN = main.c escapes.c widgets.c terminal.c mpd_functions.c draw.c input.c
OUT = -o main
LIBS = -lmpdclient main.h

run:
	$(CC) $(IN) $(OUT) $(LIBS)
