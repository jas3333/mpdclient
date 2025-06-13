#include <stdio.h>

void move_cursor(int y, int x) {
	printf("\x1b[%d;%dH", y, x);
	fflush(stdout);
}

void hide_cursor() {
	printf("\x1b[?25l");
	fflush(stdout);
}

void show_cursor() {
	printf("\x1b[?25h");
	fflush(stdout);
}

void cursor_h() {
	printf("\x1b[H");
	fflush(stdout);
}

void cls() {
	printf("\x1b[2J");
	fflush(stdout);
}

void eraseLine() {
	printf("\x1b[2K");
	fflush(stdout);
}

void setFGColor(int color) {
	printf("\x1b[38;5;%dm", color);
	fflush(stdout);
}

void resetColor() {
	printf("\x1b[39m");
	printf("\x1b[49m");
	fflush(stdout);
}

void setItalic() {
	printf("\x1b[3m");
	fflush(stdout);
}

void resetItalic() {
	printf("\x1b[23m");
	fflush(stdout);
}

void deleteToEnd() {
	printf("\x1b[0K");
	fflush(stdout);
}
void deleteToCursor() {
	printf("\x1b[1K");
	fflush(stdout);

}

void setBGColor(int color) {
	printf("\x1b[48;5;%dm", color);
	fflush(stdout);
}
