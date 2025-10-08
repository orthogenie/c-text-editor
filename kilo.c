/*** INCLUDES ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/*** DEFINES ***/

#define FAILURE -1
#define SUCCESS 0

#define CTRL_KEY(k) ((k) & 0x1f)

/*** DATA ***/

struct editorConfig {
	int screenrows;
	int screencols;
	struct termios orig_termios;
};

struct editorConfig E;

/*** FUNCTIONS ***/

void die(const char*);

void disableRawMode(void);
void enableRawMode(void);

char editorReadKey(void);
void editorRefreshScreen(void);
void editorProcessKeypress(void);
void editorDrawRows(void);
void initEditor(void);

int getWindowSize(int*, int*);
int getCursorPosition(int*, int*);

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

/*** OUTPUT ***/

void editorDrawRows(void) {
	int y;

	for (y = 0; y < E.screenrows; y++) {
		write(STDOUT_FILENO, "~", 1);

		if (y < E.screenrows - 1) {
			write(STDOUT_FILENO, "\r\n", 2);
		}
	}
}

void editorRefreshScreen(void) {
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	editorDrawRows();

	write(STDOUT_FILENO, "\x1b[H", 3);
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
