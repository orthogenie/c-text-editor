/*** INCLUDES ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/*** DEFINES ***/

#define KILO_VERSION "0.0.1"

#define FAILURE -1
#define SUCCESS 0

#define CTRL_KEY(k) ((k) & 0x1f)
#define ABUF_INIT {NULL, 0}

/*** DATA ***/

/* Config data for the editor. */
struct editorConfig {
	int cx, cy;		// Cursor position
	int screenrows;	// Number of rows
	int screencols;	// Number of columns
	struct termios orig_termios;	// termios object
};

/* Global editor object. */
struct editorConfig E;

/* String buffer object */
struct abuf {
	char* b;	// String buffer
	int len;	// String length
};

/*** FUNCTIONS ***/

/* Error handling */
void die(const char*);

/* Raw mode */
void disableRawMode(void);
void enableRawMode(void);

/* Editor */
char editorReadKey(void);
void editorRefreshScreen(void);
void editorProcessKeypress(void);
void editorDrawRows(struct abuf*);
void initEditor(void);

/* Screen */
int getWindowSize(int*, int*);
int getCursorPosition(int*, int*);

/* String buffer */
void abAppend(struct abuf*, const char*, int);
void abFree(struct abuf*);

/*** TERMINAL HELPERS ***/

/* Clear the screen and output an error message. */
void die(const char* s) {
	editorRefreshScreen();

	perror(s);
	exit(1);
}

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

/* Read and return the character from keypress input. */
char editorReadKey(void) {
	int nread;
	char c;

	while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if (nread == FAILURE && errno != EAGAIN) die("read");
	}
	return c;
}

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

/*** APPEND BUFFER ***/

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

/*** OUTPUT ***/

/* Draw each row for each row in the terminal screen. */
void editorDrawRows(struct abuf* ab) {
	int y;

	for (y = 0; y < E.screenrows; y++) {
		if (y == E.screenrows / 3) {	// First line welcome message
			char welcome[80];
			int welcomelen = snprintf(welcome, sizeof(welcome),
				"Kilo editor -- version %s", KILO_VERSION);
			
			if (welcomelen > E.screencols) welcomelen = E.screencols; // Truncate

			// Centre the welcome message
			int padding = (E.screencols - welcomelen) / 2;
			if (padding) {
				abAppend(ab, "~", 1);
				padding--;
			}
			while (padding--) abAppend(ab, " ", 1);

			abAppend(ab, welcome, welcomelen);
		} 
		else {	// Line start symbol
			abAppend(ab, "~", 1);
		}

		// Clear following? line per refresh 
		abAppend(ab, "\x1b[K", 3);

		// New line
		if (y < E.screenrows - 1) {
			abAppend(ab, "\r\n", 2);
		}
	}
}

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

/*** INPUT ***/

/* Process keypress input. */
void editorProcessKeypress(void) {
	char c = editorReadKey();

	switch (c) {
		case CTRL_KEY('q'):
			editorRefreshScreen();
			exit(SUCCESS);
			break;
	}
}

/*** INIT ***/

/* Initialise the editor. */
void initEditor(void) {
	E.cx = 0;
	E.cy = 0;
	if (getWindowSize(&E.screenrows, &E.screencols) == FAILURE) die("getWindowSize");
}

/* KILO TEXT EDITOR */
int main (void) {
	enableRawMode();
	initEditor();

	while (1) {
		editorRefreshScreen();
		editorProcessKeypress();
	}

	return SUCCESS;
}
