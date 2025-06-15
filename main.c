#include <locale.h>
#include <stdio.h>
#include <sys/select.h>
#include <term.h>
#include <unistd.h>
#include "main.h"


int main() {
	setlocale(LC_ALL, "");

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

	// Queue config
	QueueData qc;

	// Queue Index
	qc.q_index = 0;

	// Scroll offset
	qc.s_offset = 0;

	// Visible lines
	qc.vlines = y - 11;

	// Top offset
	qc.t_offset = 8;

	// Queue length
	qc.qlen = queue_len;

	draw_queue(conn, queue,  &qc);
	draw_headers();
	drawVolume(conn);
	
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
				else if (ch == 'f' || ch == 'l') mpd_run_seek_current(conn, 2, true);
				else if (ch == 'b' || ch == 'h') mpd_run_seek_current(conn, -3, true);
				else if (ch == 'j') {
					if (qc.q_index < qc.qlen - 1) {
						qc.q_index++;
					}
					if (qc.q_index - qc.s_offset >= qc.vlines / 2 && qc.s_offset + qc.vlines < qc.qlen) {
						qc.s_offset++;
					}
					draw_queue(conn, queue, &qc);
				}
				else if (ch == 'J') {
					if (qc.q_index + 10 < qc.qlen - 1) {
						qc.q_index += 10;
					}
					else qc.q_index = qc.qlen- 1;

					if (qc.q_index - qc.s_offset >= qc.vlines / 2 && qc.s_offset + qc.vlines < qc.qlen) {
						if (qc.s_offset < qc.qlen - 50) {
							qc.s_offset += 10;
						}
						else {
							qc.s_offset = qc.q_index - 50;
							qc.q_index = qc.qlen - 1;
						}
					}
					draw_queue(conn, queue, &qc);
				}
				else if (ch == 'k') {
					if (qc.q_index > 0) {
						qc.q_index--;
					}
					if (qc.q_index - qc.s_offset < qc.vlines / 2 && qc.s_offset > 0) {
						qc.s_offset--;
					} 

					draw_queue(conn, queue, &qc);
				}
				else if (ch == 'K') {
					if (qc.q_index - 10 > 0) {
						qc.q_index -= 10;
					} 
					else {
						qc.q_index = 0;
					}

					if (qc.q_index - qc.s_offset < qc.vlines / 2 && qc.s_offset > 0) {
						if (qc.s_offset < 10) qc.s_offset = 0;
						else {
							qc.s_offset -= 10;
						}
					} 

					draw_queue(conn, queue, &qc);
				}
				else if (ch == '\r') {
					mpd_run_play_pos(conn, qc.q_index);
					draw_queue(conn, queue, &qc);
				}
				else if (ch == 'G') {
					qc.q_index = qc.qlen - 1;
					qc.s_offset = qc.qlen - qc.vlines;
					draw_queue(conn, queue, &qc);
				}
				else if (ch == 'g') {
					qc.q_index = 0;
					qc.s_offset = 0;
					qc.vlines = y - 11;
					qc.t_offset = 8;
					draw_queue(conn, queue, &qc);

				}
				else if (ch == '.') {
					mpd_run_change_volume(conn, 5);
					drawVolume(conn);
				}
				else if (ch == ',') {
					mpd_run_change_volume(conn, -5);
					drawVolume(conn);

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
