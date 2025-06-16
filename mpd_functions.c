#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "main.h"


void toggle_play_pause(struct mpd_connection *conn) {
	struct mpd_status *status = mpd_run_status(conn);

	enum mpd_state state = mpd_status_get_state(status);
	mpd_status_free(status);

	if (state == MPD_STATE_PLAY) mpd_run_pause(conn, true);
	else mpd_run_play(conn);
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
