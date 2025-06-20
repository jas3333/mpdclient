#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <term.h>
#include <wchar.h>
#include "main.h"

void draw_song_stats_widget(SongStatsWidget *widget) {
	if (!widget->dirty) return;
	move_cursor(2, 20);
	deleteToCursor();

	move_cursor(2, 1);


	setFGColor(SONG_PROGRESS);
	printf("%u:%02u / %u:%02u", widget->elapsed / 60, widget->elapsed % 60, widget->total / 60, widget->total % 60);
	resetColor();

	fflush(stdout);
}

void draw_mpd_status_widget(MPDStatusWidget *widget) {
	move_cursor(1, 1);
	if (!widget->dirty) return;


	setFGColor(MPD_STATUS);
	setItalic();
	printf("%-20s", widget->status);
	widget->dirty = false;

	resetItalic();
	resetColor();

	fflush(stdout);
}

void draw_now_playing_widget(NowPlayingWidget *widget) {
	if (!widget->dirty) return;

	int y;
	int x;

	char buffer[128];

	int artistLength = strlen(widget->artist);
	int titleLength = strlen(widget->title);
	int total = artistLength + titleLength;

	move_cursor(1, 10);
	deleteToEnd();

	getCenter(&y, &x, widget->width);

	move_cursor(1, x);
	
	setFGColor(NOW_PLAYING_ARTIST);
	setItalic();
	printf("%s - ", widget->artist);
	resetColor();
	resetItalic();

	setFGColor(NOW_PLAYING_TITLE);
	printf("%s", widget->title);
	resetColor();

	fflush(stdout);

	
	widget->dirty = false;
	widget->songChange = false;
}

void draw_progress_bar(SongStatsWidget *widget) {
	int y;
	int x;

	getTerminalSize(&y, &x);
	int barWidth = x - 1;
	move_cursor(y - 2, 1);

	if (widget->total == 0) {
		widget->total = 1;
	}

	float ratio = (float)widget->elapsed / widget->total;
	int progress_width = (int)(ratio * barWidth);
	int empty_width = x - progress_width;


	setFGColor(PROGRESS_BAR);
	for (int i = 0; i < progress_width - 1; i++) {
        printf("▋");
	}

	for (int i = 0; i < empty_width; i++) {
        printf("░");
	}

	resetColor();
	fflush(stdout);
}

void draw_line(int y, int x, int width, char *block) {
	for (int i = 0; i < width; i++) {
		move_cursor(y, x + i);
		printf("%s", block);
	}
}

void draw_headers() {
	int y;
	int x;
	getTerminalSize(&y, &x);
	int total_width = x - 2;

	int w_artist	= total_width * 25 / 100; 
	int w_title		= total_width * 35 / 100;
	int w_album		= total_width * 30 / 100;
	int w_duration	= total_width * 10 / 100;

	move_cursor(6, 1);
	setFGColor(HEADER_TEXT);
	printf("%-*s %-*s %-*s %*s", w_artist, "Artist", w_title, "Title", w_album, "Album", w_duration, "Duration");

	setFGColor(HEADERS);
	draw_line(5, 1, x - 1, "⎽");
	draw_line(7, 1, x - 1, "⎺");

	fflush(stdout);
}

int utf8_display_width(const char *s) {
	int width = 0;
	mbstate_t ps = {0};
	wchar_t wc;
	size_t len;

	while (*s) {
		len = mbrtowc(&wc, s, MB_CUR_MAX, &ps);
		if (len == (size_t)-1 || len == (size_t)-2) {
			s++; 
			continue;
		}
		if (len == 0)
			break;

		int w = wcwidth(wc);
		width += (w >= 0) ? w : 0;
		s += len;
	}
	return width;
}

void print_aligned(const char *s, int field_width) {
	int w = utf8_display_width(s);
	printf("%s", s);
	for (int i = 0; i < field_width - w; i++) {
		putchar(' ');
	}
}

void draw_queue(struct mpd_connection *conn,  SongEntry *queue, QueueData *qconfig) {

	struct mpd_status *status = mpd_run_status(conn);
	int song_pos = mpd_status_get_song_pos(status);
	mpd_status_free(status);

	int y;
	int x;
	getTerminalSize(&y, &x);
	int total_width = x - 2;

	int w_artist	= total_width * 25 / 100; 
	int w_title		= total_width * 35 / 100;
	int w_album		= total_width * 30 / 100;
	int w_duration  = total_width * 10 / 100;
	resetColor();

	for (int i = 0; i < qconfig->vlines; i++) {
		int song_index = qconfig->s_offset + i;
		move_cursor(qconfig->t_offset + i, 1);
		eraseLine();

		if (song_index < qconfig->qlen) {
			char buffer[16];
			snprintf(buffer, sizeof(buffer), "%2u:%02u", queue[song_index].duration / 60, queue[song_index].duration % 60);

			if (song_index == qconfig->q_index) {
				setItalic();
				setBGColor(SELECTOR_BAR_BG);
				setFGColor(SELECTOR_BAR_FG);
			}

			else setFGColor(QUEUE_SONGS_FG);



			if (song_index == song_pos && song_index != qconfig->q_index) {
				setFGColor(ACTIVE_SONG_QUEUE);
				print_aligned(queue[song_index].artist,  w_artist);
				putchar(' ');
				print_aligned(queue[song_index].title,   w_title);
				putchar(' ');
				print_aligned(queue[song_index].album,   w_album);
				putchar(' ');
				printf("%*s", w_duration, buffer);

				resetColor();
				continue;
			}


			print_aligned(queue[song_index].artist,  w_artist);
			putchar(' ');
			print_aligned(queue[song_index].title,   w_title);
			putchar(' ');
			print_aligned(queue[song_index].album,   w_album);
			putchar(' ');
			printf("%*s", w_duration, buffer);

			if (song_index == qconfig->q_index) {
				resetItalic();
				resetColor();
				resetModes();
			}
		}
	}
}

void drawVolume(struct mpd_connection *connection) {
	int volume = mpd_run_get_volume(connection);
	int volumeBlockCount = abs(volume / 10);
	int shadeBlockCount = 10 - volumeBlockCount;

	char *volumeBlock = "▊";
	char *shadeBlock = "░";

	setFGColor(VOLUME);
	moveCursorX(4, 89);
	deleteToEnd();

	printf("Volume: ");
	for (int i = 0; i < volumeBlockCount; i++) {
		printf("%s", volumeBlock);
	}
	for (int i = 0; i < shadeBlockCount; i++) {
		printf("%s", shadeBlock);
	}

	printf(" %d%%", volume);

	resetColor();
}

void drawDirectoryHeader() {
	int y;
	int x;
	getTerminalSize(&y, &x);
	int total_width = x - 2;

	move_cursor(6, 1);

	setFGColor(DIRECTORY_HEADER_TEXT);
	printf("Directories:");
	resetColor();

	setFGColor(DIRECTORY_HEADER);
	draw_line(5, 1, x - 1, "⎽");
	draw_line(7, 1, x - 1, "⎺");

	resetColor();

	fflush(stdout);

}

void clearViewArea() {
	int y, x;
	getTerminalSize(&y, &x);

	int visibleArea = y - 8;

	for (int i = 0; i < visibleArea; i++) {
		move_cursor(5 + i, 1);
		deleteToEnd();
	}
}

void displayEntries(Entry *entries, DirState *state) {
	resetColor();

	int y, x;
	getTerminalSize(&y, &x); 
	int totalWidth = x - 1;

	for (int i = 0; i < state->vlines; i++) {
		int index = state->s_offset + i;
		move_cursor(8 + i, 1);
		eraseLine();

		if (index >= state->entryCount) continue;
		if (index == state->selected) {
			setBGColor(DIRECTORY_SELECTOR_BG);
			setFGColor(DIRECTORY_SELECTOR_FG);
		}
		else setFGColor(DIRECTORY_TEXT);

		char buffer[512];
		strncpy(buffer, entries[index].name, sizeof(buffer));
		char *lastToken = NULL;
		char *token = strtok(buffer, "/");
		while (token != NULL) {
			lastToken = token;
			token = strtok(NULL, "/");
		}

		printf("%-*s", totalWidth, lastToken);

		if (index == state->selected) resetColor();
	}
}
