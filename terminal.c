#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "main.h"

struct termios original; 

void restore_terminal() {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);

	cls();
	cursor_h();
	show_cursor();
}

void initTerm() {
	tcgetattr(STDIN_FILENO, &original);

	atexit(restore_terminal);

	struct termios new_term = original;
	cfmakeraw(&new_term);

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_term);

	cls();
	cursor_h();
	hide_cursor();
}

void getTerminalSize(int *y, int *x) {
	struct winsize win;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);

	*y = win.ws_row;
	*x = win.ws_col;
}

void findCursor(int *y, int *x) {
	char buffer[32];
	int i = 0;
	write(STDOUT_FILENO, "\x1b[6n", 4);

	while (i < sizeof(buffer)) {
		if (read(STDIN_FILENO, &buffer[i], 1) == 0) break;
		if (buffer[i] == 'R') break;
		i++;
	}

	buffer[i] = '\0';
	sscanf(&buffer[2], "%d;%d", y, x);
}

void getCenter(int *y, int *x, int length) {
	int yMax;
	int xMax;

	getTerminalSize(&yMax, &xMax);

	int centerY = yMax / 2;
	int centerX = (xMax - length) / 2;

	*y = centerY;
	*x = centerX;
}
