#include <stdio.h>
#include <unistd.h>
#include "main.h"


void toggle_play_pause(struct mpd_connection *conn) {
	struct mpd_status *status = mpd_run_status(conn);

	enum mpd_state state = mpd_status_get_state(status);
	mpd_status_free(status);

	if (state == MPD_STATE_PLAY) mpd_run_pause(conn, true);
	else mpd_run_play(conn);
}

