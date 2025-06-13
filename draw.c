#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <term.h>
#include "main.h"

void draw_song_stats_widget(SongStatsWidget *widget) {
	if (!widget->dirty) return;
	move_cursor(2, 20);
	deleteToCursor();

	move_cursor(2, 1);


	printf("%u:%02u / %u:%02u", widget->elapsed / 60, widget->elapsed % 60, widget->total / 60, widget->total % 60);

	fflush(stdout);
}

void draw_mpd_status_widget(MPDStatusWidget *widget) {
	if (!widget->dirty) return;

	move_cursor(1, 1);

	setFGColor(12);
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
	
	setFGColor(218);
	setItalic();
	printf("%s - ", widget->artist);
	resetColor();
	resetItalic();
	printf("%s", widget->title);

	fflush(stdout);

	
	widget->dirty = false;
}

void draw_progress_bar(SongStatsWidget *widget) {
	int y;
	int x;

	getTerminalSize(&y, &x);
	int barWidth = x - 1;
	move_cursor(y - 2, 1);
	eraseLine();

	if (widget->total == 0) {
		widget->total = 1;
	}

	float ratio = (float)widget->elapsed / widget->total;
	int progress_width = (int)(ratio * barWidth);
	int empty_width = x - progress_width;


	setFGColor(12);
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
	printf("%-*s %-*s %-*s %*s", w_artist, "Artist", w_title, "Title", w_album, "Album", w_duration, "Duration");

	setFGColor(12);
	draw_line(5, 1, x - 1, "⎽");
	draw_line(7, 1, x - 1, "⎺");

	fflush(stdout);
}

void draw_queue(struct mpd_connection *conn, int start_index, int height, int y_offset, int queue_len, SongEntry *queue, int selected_index) {

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

	for (int i = 0; i < height; i++) {
		int song_index = start_index + i;
		move_cursor(y_offset + i, 1);
		eraseLine();

		if (song_index < queue_len) {
			char buffer[16];
			snprintf(buffer, sizeof(buffer), "%2u:%02u", queue[song_index].duration / 60, queue[song_index].duration % 60);

			if (song_index == selected_index) {
				setItalic();
				setBGColor(12);
				setFGColor(0);
			}

			if (song_index == song_pos && song_index != selected_index) {
				setFGColor(75);
				printf("%-*s %-*s %-*s %*s", w_artist, queue[song_index].artist, w_title, queue[song_index].title, w_album, queue[song_index].album, w_duration, buffer);
				resetColor();
				continue;
			}

			printf("%-*s %-*s %-*s %*s", w_artist, queue[song_index].artist, w_title, queue[song_index].title, w_album, queue[song_index].album, w_duration, buffer);

			if (song_index == selected_index) {
				resetItalic();
				resetColor();
			}
		}
	}
}
