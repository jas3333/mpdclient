#include <mpd/client.h>
#include <mpd/tag.h>
#include <stdio.h>
#include <string.h>
#include "main.h"

void update_song_stats_widget(struct mpd_connection *conn, SongStatsWidget *widget) {
	struct mpd_status *status = mpd_run_status(conn);
	if (!status) {
		fprintf(stderr, "Failed to get status. songstats\n");
		return;
	}

	unsigned int elapsed = mpd_status_get_elapsed_time(status);
	unsigned int total = mpd_status_get_total_time(status);
	if (widget->elapsed != elapsed || widget->total != total) {
		widget->elapsed = elapsed;
		widget->total = total;
		widget->dirty = true;

	}

	mpd_status_free(status);
	
}

void update_now_playing_widget(struct mpd_connection *conn, NowPlayingWidget *widget) {
	struct mpd_song *song = mpd_run_current_song(conn);
	if (!song) {
		return;
	}

	const char *title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
	const char *artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);

	strncpy(widget->title, title ? title : "Unknown Title", sizeof(widget->title));
	widget->dirty = true;

	strncpy(widget->artist, artist ? artist : "Unknown Artist", sizeof(widget->artist));
	widget->dirty = true;

	int totalLength = strlen(widget->title) + strlen(widget->artist);
	widget->prevWidth = widget->width;
	widget->width = totalLength;

	mpd_song_free(song);
}

void update_mpd_status_widget(struct mpd_connection *conn, MPDStatusWidget *widget) {
	struct mpd_status *status = mpd_run_status(conn);
	if (!status) {
		fprintf(stderr, "Failed to get statuslakjsdlfkajsdlfk\n");
		return;
	}

	enum mpd_state state = mpd_status_get_state(status);
	char buffer[120];

	if (state == MPD_STATE_PLAY)		strncpy(buffer, "Playing", sizeof(buffer));
	else if (state == MPD_STATE_STOP)	strncpy(buffer, "Stopped", sizeof(buffer));
	else if (state == MPD_STATE_PAUSE)	strncpy(buffer, "Paused", sizeof(buffer));
	else strncpy(buffer, "Unknown", sizeof(buffer));

	if (strcmp(widget->status, buffer) != 0) {
		strncpy(widget->status, buffer, sizeof(widget->status));
		widget->dirty = true;
	}

	mpd_status_free(status);
}

void update_mpd_queue_widget(struct mpd_connection *conn) {
	int y;
	int x;
	getTerminalSize(&y, &x);
	int yCount = 4;
	int yCountMax = y - 10;


	mpd_send_list_queue_meta(conn);

	struct mpd_song *song;
	while ((song = mpd_recv_song(conn)) != NULL && yCount <= yCountMax) {
		const char *title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
		const char *artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);

		if (title && artist) {
			move_cursor(yCount, 1);
			printf("%s - %s", title, artist);
		}

		yCount++;
		mpd_song_free(song);
	}

}

void load_queue(struct mpd_connection *conn, SongEntry *queue, QueueData *qc) {
	int queue_index = 0;

	mpd_send_list_queue_meta(conn);

	struct mpd_song *song;
	while ((song = mpd_recv_song(conn)) != NULL && queue_index < MAX_SONGS) {
		const char *title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
		const char *artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
		const char *album = mpd_song_get_tag(song, MPD_TAG_ALBUM, 0);
		queue[queue_index].duration = mpd_song_get_duration(song);

		strncpy(queue[queue_index].title, title ? title : "Unknown Title", MAX_TITLE_LEN - 1);
		strncpy(queue[queue_index].artist, artist ? artist : "Unknown Artist", MAX_ARTIST_LEN - 1);
		strncpy(queue[queue_index].album, album ? album : "Unknown Album", MAX_ALBUM_LEN - 1);

		queue[queue_index].title[MAX_TITLE_LEN - 1] = '\0';
		queue[queue_index].artist[MAX_ARTIST_LEN - 1] = '\0';
		queue[queue_index].album[MAX_ALBUM_LEN - 1] = '\0';
		mpd_song_free(song);
		queue_index++;
	}

	qc->qlen = queue_index;

	mpd_response_finish(conn);
}


