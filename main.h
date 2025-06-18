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
#define MAX_DEPTH 10
#define MAX_ENTRIES 1024


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

typedef struct {
	char name[512];
	bool is_dir;
} Entry;

typedef struct {
	int selected;
	int s_offset;
	int vlines;
	int entryCount;
	int depth;
	int previous[MAX_DEPTH];
	char pathStack[MAX_DEPTH][512];
} DirState;

typedef struct {
	int duration;
	time_t startTime;
	bool display;
	char message[1024];

} Alert;

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
void deleteToBegin(); 

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
void load_queue(struct mpd_connection *conn, SongEntry *queue, QueueData *qc); 

// draw.c
void draw_song_stats_widget(SongStatsWidget *widget);
void draw_mpd_status_widget(MPDStatusWidget *widget); 
void draw_now_playing_widget(NowPlayingWidget *widget); 
void draw_progress_bar(SongStatsWidget *widget); 
void draw_queue(struct mpd_connection *conn, SongEntry *queue, QueueData *qc); 
void draw_headers();
void drawVolume(struct mpd_connection *connection); 
void clearViewArea(); 
void drawDirectoryHeader(); 
void displayEntries(Entry *entries, DirState *state); 

// Mpd Functions
void toggle_play_pause(struct mpd_connection *conn, QueueData *qc); 
int listDirectory(struct mpd_connection *connection, const char *path, Entry *entries); 
void setAlert(Alert *alert, const char *message, int duration); 
void addSelectedToQueue(struct mpd_connection *connection, DirState *state, Entry *entries, QueueData *qc, Alert *alert); 
void drawAlert(Alert *alert); 
void clearQueue(struct mpd_connection *connection, SongEntry *queue, QueueData *qc); 

// input.c
void handleNavUp(struct mpd_connection *connection, QueueData *qc, SongEntry *queue); 
void handleNavDown(struct mpd_connection *connection, QueueData *qc, SongEntry *queue); 
void handlePageDown(struct mpd_connection *connection, QueueData *qc, SongEntry *queue); 
void handlePageUp(struct mpd_connection *connection, QueueData *qc, SongEntry *queue); 
void playSelected(struct mpd_connection *connection, QueueData *qc, SongEntry *queue); 
void jumpToBottom(struct mpd_connection *connection, QueueData *qc, SongEntry *queue); 
void jumpToTop(struct mpd_connection *connection, QueueData *qc, SongEntry *queue); 
void volumeUp(struct mpd_connection *connection); 
void volumeDown(struct mpd_connection *connection); 
void directoryNavDown(DirState *state, Entry *entries); 
void directoryNavUp(DirState *state, Entry *entries); 
void directoryNavForward(struct mpd_connection *connection, DirState *state, Entry *entries); 
void directoryNavBack(struct mpd_connection *connection, DirState *state, Entry *entries); 

#endif
