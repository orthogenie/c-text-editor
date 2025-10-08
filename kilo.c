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

struct editorConfig {
	int screenrows;
	int screencols;
	struct termios orig_termios;
};

struct editorConfig E;

struct abuf {
	char* b;
	int len;
};

/*** FUNCTIONS ***/

void die(const char*);

void disableRawMode(void);
void enableRawMode(void);

char editorReadKey(void);
void editorRefreshScreen(void);
void editorProcessKeypress(void);
void editorDrawRows(struct abuf*);
void initEditor(void);

int getWindowSize(int*, int*);
int getCursorPosition(int*, int*);

void abAppend(struct abuf*, const char*, int);
void abFree(struct abuf*);

/*** TERMINAL HELPERS ***/

void die(const char* s) {
	editorRefreshScreen();

	perror(s);
	exit(1);
}

void disableRawMode(void) {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == FAILURE) {
		die("tcsetattr");
	}
}

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

char editorReadKey(void) {
	int nread;
	char c;

	while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if (nread == FAILURE && errno != EAGAIN) die("read");
	}
	return c;
}

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

/* Reassign memory for buffer. */
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
		if (y == E.screenrows / 3) { // First line welcome message
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
		else { // Line start symbol
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

void editorRefreshScreen(void) {
	struct abuf ab = ABUF_INIT;
	
	abAppend(&ab, "\x1b[?25l", 6);
	// abAppend(&ab, "\x1b[2J", 4);
	abAppend(&ab, "\x1b[H", 3);

	editorDrawRows(&ab);

	abAppend(&ab, "\x1b[H", 3);
	abAppend(&ab, "\x1b[?25h", 6);

	write(STDOUT_FILENO, ab.b, ab.len);
	abFree(&ab);
}

/*** INPUT ***/

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

void initEditor(void) {
	if (getWindowSize(&E.screenrows, &E.screencols) == FAILURE) die("getWindowSize");
}

int main (void) {
	enableRawMode();
	initEditor();

	while (1) {
		editorRefreshScreen();
		editorProcessKeypress();
	}

	return SUCCESS;
}
