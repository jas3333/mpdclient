#include <locale.h>
#include <stdio.h>
#include <sys/select.h>
#include <term.h>
#include <unistd.h>
#include "main.h"

typedef enum {
	MODE_QUEUE,
	MODE_BROWSER
} AppMode;

int main() {

	setlocale(LC_ALL, "");

	Alert alerts = {0};

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

	NowPlayingWidget	nowPlaying	= { .dirty = true, .songChange = false };
	MPDStatusWidget		mpdStatus	= { .dirty = true };
	SongStatsWidget		songStatus	= { .dirty = true };

	SongEntry queue[MAX_SONGS];
	int queue_len = 0;

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

	load_queue(conn, queue, &qc);
	draw_queue(conn, queue,  &qc);
	draw_headers();
	drawVolume(conn);
	
	// Browser Mode Items
	AppMode mode = MODE_QUEUE;
	Entry entries[1024];

	DirState nav = { .depth = 0, .selected = 0, .entryCount = 0, .vlines = y - 11};
	nav.entryCount = listDirectory(conn, NULL, entries); 

	char ch = 0;
	while (1) {

		fd_set readfds;
		struct timeval timeout;

		FD_ZERO(&readfds);
		FD_SET(STDIN_FILENO, &readfds);

		timeout.tv_sec = 0;
		timeout.tv_usec = 32000;
		int ready = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

		if (FD_ISSET(STDIN_FILENO, &readfds)) {
			if (read(STDIN_FILENO, &ch, 1) > 0) {
				if (ch == 'q') break;
				switch (mode) {
					case MODE_QUEUE: 
						switch (ch) {
							case '\t': 
								mode = MODE_BROWSER; 
								clearViewArea(); 
								drawDirectoryHeader();
								displayEntries(entries, &nav);
								break;
							case 'o': 
								mode = MODE_BROWSER; 
								clearViewArea(); 
								drawDirectoryHeader();
								displayEntries(entries, &nav);
								break;
							case 'p' : toggle_play_pause(conn, &qc); break;
							case 'f' : seekForward(conn, &mpdStatus); break;
							case 'b' : seekBack(conn, &mpdStatus); break;
							case 'l' : seekForward(conn, &mpdStatus); break;
							case 'h' : seekBack(conn, &mpdStatus); break;
							case 'j' : handleNavDown(conn, &qc, queue); break;
							case 'J' : handlePageDown(conn, &qc, queue); break;
							case 'k' : handleNavUp(conn, &qc, queue); break;
							case 'K' : handlePageUp(conn, &qc, queue); break;
							case '\r': playSelected(conn, &qc, queue); break;
							case 'G' : jumpToBottom(conn, &qc, queue); break;
							case 'g' : jumpToTop(conn, &qc, queue); break;
							case '.' : volumeUp(conn); break;
							case ',' : volumeDown(conn); break;
							case 'd': 
								mpd_run_delete(conn, qc.q_index);
								load_queue(conn, queue, &qc);
								draw_queue(conn, queue, &qc); 
								break;
							case 'n': 
								mpd_run_delete(conn, qc.q_index);
								load_queue(conn, queue, &qc);
								draw_queue(conn, queue, &qc); 
								break;
							case 'D': 
								clearQueue(conn, queue, &qc);
								break;
							default: break;
						}
					break;
					case MODE_BROWSER: 

						switch (ch) {
							case '\t': 
								mode = MODE_QUEUE; 
								clearViewArea();
								draw_headers(); 
								draw_queue(conn, queue, &qc); 
								break;
							case 'o': 
								mode = MODE_QUEUE; 
								clearViewArea();
								draw_headers(); 
								draw_queue(conn, queue, &qc); 
								break;
							case '.' : volumeUp(conn); break;
							case ',' : volumeDown(conn); break;
							case 'p' : toggle_play_pause(conn, &qc); break;
							case 'f' : seekForward(conn, &mpdStatus); break;
							case 'b' : seekBack(conn, &mpdStatus); break;
							case 'j': directoryNavDown(&nav, entries); break; 
							case 'k': directoryNavUp(&nav, entries); break; 
							case 'l': directoryNavForward(conn, &nav, entries); break;
							case 'h': directoryNavBack(conn, &nav, entries); break;
							case 'a': 
								addSelectedToQueue(conn, &nav, entries, &qc, &alerts);
								load_queue(conn, queue, &qc);
								break;
							case 'i': 
								addSelectedToQueue(conn, &nav, entries, &qc, &alerts);
								load_queue(conn, queue, &qc);
								break;
							default: break;
						}
					break;
				}
			}
		}

		move_cursor(4, 20);
		deleteToCursor();
		move_cursor(4, 1);
		setFGColor(TRACK_COUNTER);
		printf("Tracks: %d/%d", qc.q_index + 1, qc.qlen);
		resetColor();


		update_now_playing_widget(conn, &nowPlaying);
		if (nowPlaying.songChange == true) draw_queue(conn, queue, &qc);
		draw_now_playing_widget(&nowPlaying);

		update_mpd_status_widget(conn, &mpdStatus);
		draw_mpd_status_widget(&mpdStatus);
		draw_progress_bar(&songStatus); 

		update_song_stats_widget(conn, &songStatus);
		draw_song_stats_widget(&songStatus);
		drawAlert(&alerts);
	}

	return 0;
}
