#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "main.h"


void toggle_play_pause(struct mpd_connection *conn, QueueData *qc) {
	if (qc->qlen > 0) {
		struct mpd_status *status = mpd_run_status(conn);

		enum mpd_state state = mpd_status_get_state(status);
		mpd_status_free(status);

		if (state == MPD_STATE_PLAY) mpd_run_pause(conn, true);
		else mpd_run_play(conn);
	}
}

int listDirectory(struct mpd_connection *connection, const char *path, Entry *entries) {
	mpd_send_list_meta(connection, path);

	int count = 0;

	struct mpd_entity *entity;
	while ((entity = mpd_recv_entity(connection)) != NULL || count == MAX_ENTRIES - 1) {
		enum mpd_entity_type type = mpd_entity_get_type(entity);

		switch (type) {
			case MPD_ENTITY_TYPE_DIRECTORY: {
				const char *dirPath = mpd_directory_get_path(mpd_entity_get_directory(entity));
				strncpy(entries[count].name, dirPath, sizeof(entries[count].name));
				entries[count].is_dir = true;
				count++;
				break;
			}
			case MPD_ENTITY_TYPE_SONG: {
				const char *filePath = mpd_song_get_uri(mpd_entity_get_song(entity));
				strncpy(entries[count].name, filePath, sizeof(entries[count].name));
				entries[count].is_dir = false;
				count++;
				break;
			}
			default:
				break;
		}

		mpd_entity_free(entity);
	}

	mpd_response_finish(connection);
	return count;
}

void setAlert(Alert *alert, const char *message, int duration) {
	alert->duration = duration;
	alert->startTime = time(NULL);
	alert->display = true;
	snprintf(alert->message, sizeof(alert->message), "%s", message);
}

void drawAlert(Alert *alert) {
	if (!alert->display) return;

	int y, x;
	int centerY, centerX;
	int messageLength = strlen(alert->message);

	getTerminalSize(&y, &x);
	getCenter(&centerY, &centerX, messageLength);

	move_cursor(y, 1);
	deleteToEnd();

	time_t timeNow = time(NULL);
	if (timeNow - alert->startTime >= alert->duration) {
		alert->display = false;
		move_cursor(y, 1);
		deleteToEnd();
	}
	else {
		alert->display = true;
		move_cursor(y, centerX);
		setFGColor(ADD_TO_QUEUE);
		printf("%s", alert->message);
		fflush(stdout);
	}

	resetColor();
}

void addSelectedToQueue(struct mpd_connection *connection, DirState *state, Entry *entries, QueueData *qc, Alert *alert) {
	mpd_run_add(connection, entries[state->selected].name);
	char buffer[1024];
	snprintf(buffer, sizeof(buffer), "🔔 Added to queue: %s", entries[state->selected].name);
	setAlert(alert, buffer, 5);
}

void clearQueue(struct mpd_connection *connection, SongEntry *queue, QueueData *qc) {
	mpd_run_clear(connection);
	load_queue(connection, queue, qc);
	qc->qlen = 0;
	qc->s_offset = 0;
	qc->q_index = 0;
	draw_queue(connection, queue, qc);
}

void seekForward(struct mpd_connection *conn, MPDStatusWidget *widget) {
	if (strcmp(widget->status, "Playing") == 0 || strcmp(widget->status, "Paused") == 0) {
		mpd_run_seek_current(conn, 2, true);
	}
}
void seekBack(struct mpd_connection *conn, MPDStatusWidget *widget) {
	if (strcmp(widget->status, "Playing") == 0 || strcmp(widget->status, "Paused") == 0) {
		mpd_run_seek_current(conn, -3, true);
	}
}
