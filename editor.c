#include "editor.h"
#include "io_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

/****************/
/*** RAW MODE ***/
/****************/

/* Disable raw mode. */
void disableRawMode(void) {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == FAILURE) {
		die("tcsetattr");
	}
}

/* Enable raw mode. */
void enableRawMode(void) {
	if (tcgetattr(STDIN_FILENO, &E.orig_termios) == FAILURE) die("tcgetattr");
	atexit(disableRawMode);
	
	struct termios raw = E.orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag &= ~(CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == FAILURE) die("tcsetattr");
}

/**********************/
/*** SCREEN HELPERS ***/
/**********************/

/* Get cursor position. */
int getCursorPosition(int* rows, int* cols) {
	char buf[32];
	unsigned int i = 0;
	
	if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return FAILURE;

	while (i < sizeof(buf) - 1) {
		if (read(STDIN_FILENO, &buf[i], 1) != 1) return FAILURE;
		if (buf[i] == 'R') break;
		i++;
	}

	buf[i] = '\0';

	if (buf[0] != '\x1b' || buf[1] != '[') return FAILURE;
	if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return FAILURE;

	return SUCCESS;
}

/* Get terminal window size and save to given pointers. */
int getWindowSize(int* rows, int* cols) {
	struct winsize ws;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == FAILURE || ws.ws_col == 0) {
		// 999 arbitary large number
		if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return FAILURE;
		
		return getCursorPosition(rows, cols);
	} else {
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return SUCCESS;
	}
}

/***********************/
/*** TERMINAL SCREEN ***/
/***********************/

/* Draws (writes) to the terminal screen. */
void editorRefreshScreen(void) {
	struct abuf ab = ABUF_INIT;
	
	abAppend(&ab, "\x1b[?25l", 6);	// Hide cursor
	abAppend(&ab, "\x1b[H", 3);		// Position cursor to the start

	editorDrawRows(&ab);			// Draw each line

	abAppend(&ab, "\x1b[H", 3); 	// Position cursor to the start
	abAppend(&ab, "\x1b[?25h", 6); 	// Show cursor

	// Write to terminal
	write(STDOUT_FILENO, ab.b, ab.len);
	abFree(&ab);
}

/* Clear the screen and output an error message. */
void die(const char* s) {
	editorRefreshScreen();

	perror(s);
	exit(1);
}

/************/
/*** INIT ***/
/************/

/* Initialise the editor. */
void initEditor(void) {
	E.cx = 0;
	E.cy = 0;
	if (getWindowSize(&E.screenrows, &E.screencols) == FAILURE) die("getWindowSize");
}
