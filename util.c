#include "util.h"

#include "editor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
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

/* Clear the screen and output an error message. */
void die(const char* s) {
	editorRefreshScreen();

	perror(s);
	exit(1);
}


/*********************/
/*** APPEND BUFFER ***/
/*********************/


/* Appends the message buffer with a provided string. Reassigns memory as needed. */
void abAppend(struct abuf* ab, const char* s, int len) {
	char* new = realloc(ab->b, ab->len + len);

	if (new == NULL) return;
	memcpy(&new[ab->len], s, len);
	ab->b = new;
	ab->len += len;
}

/* Free buffer memory. */
void abFree(struct abuf* ab) {
	free(ab->b);
}
