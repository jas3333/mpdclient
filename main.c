#include <stdio.h>
#include <sys/select.h>
#include <term.h>
#include <unistd.h>
#include "main.h"

int main() {

	initTerm();
	hide_cursor();
	cls();

	int y, x;
	getTerminalSize(&y, &x);

	struct mpd_connection *conn = mpd_connection_new(NULL, 0, 30000);
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
		fprintf(stderr, "failed to connect to MPD\n");
		return 1;
	}

	NowPlayingWidget	nowPlaying	= { .dirty = true };
	MPDStatusWidget		mpdStatus	= { .dirty = true };
	SongStatsWidget		songStatus	= { .dirty = true };

	SongEntry queue[MAX_SONGS];
	int queue_len = 0;
	load_queue(conn, queue, &queue_len);


	int q_index = 0;
	int scroll_offset = 0;
	int visible_lines = y - 11;
	int top_offset = 8;
	draw_queue(conn, q_index, visible_lines, top_offset, queue_len, queue, q_index);
	draw_headers();
	
	char ch = 0;
	while (1) {

		fd_set readfds;
		struct timeval timeout;

		FD_ZERO(&readfds);
		FD_SET(STDIN_FILENO, &readfds);

		timeout.tv_sec = 0;
		timeout.tv_usec = 16000;
		int ready = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

		if (FD_ISSET(STDIN_FILENO, &readfds)) {
			if (read(STDIN_FILENO, &ch, 1) > 0) {
				if (ch == 'q') {
					break;
				}
				else if (ch == 'p') toggle_play_pause(conn);
				else if (ch == 'f') mpd_run_seek_current(conn, 2, true);
				else if (ch == 'b') mpd_run_seek_current(conn, -3, true);
				else if (ch == 'j') {
					if (q_index < queue_len - 1) {
						q_index++;
					}

					if (q_index - scroll_offset >= visible_lines / 2 && scroll_offset + visible_lines < queue_len) {
						scroll_offset++;
					}
					draw_queue(conn, scroll_offset, visible_lines, top_offset, queue_len, queue,q_index );
				}
				else if (ch == 'k') {
					if (q_index > 0) {
						q_index--;
					}
					if (q_index - scroll_offset < visible_lines / 2 && scroll_offset > 0) {
						scroll_offset--;
					} 

					draw_queue(conn, scroll_offset, visible_lines, top_offset, queue_len, queue, q_index);
				}
				else if (ch == '\r') {
					mpd_run_play_pos(conn, q_index);
					draw_queue(conn, scroll_offset, visible_lines, top_offset, queue_len, queue,q_index );
				}
				else if (ch == 'G') {
					q_index = queue_len - 1;
					scroll_offset = queue_len - visible_lines;
					draw_queue(conn, scroll_offset, visible_lines, top_offset, queue_len, queue, q_index);
				}
			}
		}

		update_now_playing_widget(conn, &nowPlaying);
		draw_now_playing_widget(&nowPlaying);

		update_mpd_status_widget(conn, &mpdStatus);
		draw_mpd_status_widget(&mpdStatus);
		draw_progress_bar(&songStatus); 

		update_song_stats_widget(conn, &songStatus);
		draw_song_stats_widget(&songStatus);
	}

	return 0;
}
