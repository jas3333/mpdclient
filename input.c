#include "main.h"

void handleNavUp(struct mpd_connection *connection, QueueData *qc, SongEntry *queue) {
	if (qc->q_index < qc->qlen - 1) qc->q_index++; 
	if (qc->q_index - qc->s_offset >= qc->vlines / 2 && qc->s_offset + qc->vlines < qc->qlen) qc->s_offset++;
	draw_queue(connection, queue, qc);
}


void handleNavDown(struct mpd_connection *connection, QueueData *qc, SongEntry *queue) {
	if (qc->q_index > 0) qc->q_index--;
	if (qc->q_index - qc->s_offset < qc->vlines / 2 && qc->s_offset > 0) qc->s_offset--;
	draw_queue(connection, queue, qc);
}

void handlePageDown(struct mpd_connection *connection, QueueData *qc, SongEntry *queue) {
	if (qc->q_index + 10 < qc->qlen - 1) qc->q_index += 10;
	else qc->q_index = qc->qlen - 1;

	if (qc->q_index - qc->s_offset >= qc->vlines / 2 && qc->s_offset + qc->vlines < qc->qlen) {
		if (qc->s_offset < qc->qlen - 50) qc->s_offset += 10;
		else {
			qc->s_offset = qc->q_index - 50;
			qc->q_index = qc->qlen - 1;
		}
	}
	draw_queue(connection, queue, qc);
}

void handlePageUp(struct mpd_connection *connection, QueueData *qc, SongEntry *queue) {
	if (qc->q_index - 10 > 0) qc->q_index -= 10;
	else qc->q_index = 0;

	if (qc->q_index - qc->s_offset < qc->vlines / 2 && qc->s_offset > 0) {
		if (qc->s_offset < 10) qc->s_offset = 0;
		else qc->s_offset -= 10;
	}
	draw_queue(connection, queue, qc);
}

void playSelected(struct mpd_connection *connection, QueueData *qc, SongEntry *queue) {
	mpd_run_play_pos(connection, qc->q_index);
	draw_queue(connection, queue, qc);
}

void jumpToBottom(struct mpd_connection *connection, QueueData *qc, SongEntry *queue) {
	qc->q_index = qc->qlen - 1;
	qc->s_offset = qc->qlen - qc->vlines;
	draw_queue(connection, queue, qc);
}
void jumpToTop(struct mpd_connection *connection, QueueData *qc, SongEntry *queue) {
	int y, x;
	getTerminalSize(&y, &x);

	qc->q_index = 0;
	qc->s_offset = 0;
	qc->vlines = y - 11;
	qc->t_offset = 8;
	draw_queue(connection, queue, qc);
}

void volumeUp(struct mpd_connection *connection) {
	mpd_run_change_volume(connection, 5);
	drawVolume(connection);
}

void volumeDown(struct mpd_connection *connection) {
	mpd_run_change_volume(connection, -5);
	drawVolume(connection);
}

