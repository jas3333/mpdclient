#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>
#include <mpd/client.h>
#include <wchar.h>
#include <locale.h>

#define MAX_TITLE_LEN 64 
#define MAX_ARTIST_LEN 40 
#define MAX_ALBUM_LEN 48
#define MAX_SONGS 10240


typedef struct {
	char title[MAX_TITLE_LEN];
	char artist[MAX_ARTIST_LEN];
	char album[MAX_ALBUM_LEN];
	unsigned int duration;
} SongEntry;

// Structs
typedef struct {
	char artist[128];
	char title[128];
	int width;
	int prevWidth;
	bool dirty;
} NowPlayingWidget;

typedef struct {
	char status[128];
	bool dirty;
} MPDStatusWidget;

typedef struct {
	unsigned int elapsed;
	unsigned int total;
	bool dirty;
} SongStatsWidget;

typedef struct {
	char **queueList;
	int height;
	bool dirty;
} QueueWidget;

typedef struct {
	int q_index;
	int s_offset;
	int vlines;
	int t_offset;
	int qlen;
} QueueData;

// Escapes.c
void cls(); 
void cursor_h(); 
void hide_cursor(); 
void show_cursor(); 
void move_cursor(int y, int x); 
void eraseLine(); 
void setFGColor(int color); 
void resetColor();
void setItalic(); 
void resetItalic(); 
void deleteToEnd(); 
void deleteToCursor(); 
void setBGColor(int color); 
void setBold(); 
void resetModes(); 
void moveCursorX(int yCoord, int percent); 

// Terminal.c
void initTerm(); 
void restore_terminal(); 
void findCursor(int *y, int *x); 
void getTerminalSize(int *y, int *x); 
void getCenter(int *y, int *x, int length); 

// widgets.c
void update_song_stats_widget(struct mpd_connection *conn, SongStatsWidget *widget);
void update_mpd_status_widget(struct mpd_connection *conn, MPDStatusWidget *widget); 
void update_now_playing_widget(struct mpd_connection *conn, NowPlayingWidget *widget); 
void load_queue(struct mpd_connection *conn, SongEntry *queue, int *queue_len); 

// draw.c
void draw_song_stats_widget(SongStatsWidget *widget);
void draw_mpd_status_widget(MPDStatusWidget *widget); 
void draw_now_playing_widget(NowPlayingWidget *widget); 
void draw_progress_bar(SongStatsWidget *widget); 
void draw_queue(struct mpd_connection *conn, SongEntry *queue, QueueData *qc); 
void draw_headers();

// Mpd Functions
void toggle_play_pause(struct mpd_connection *conn); 

#endif
