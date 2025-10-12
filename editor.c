#include "editor.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/************/
/*** INIT ***/
/************/


/* Initialise the editor. */
void initEditor(void) {
	E.cx = 0;
	E.cy = 0;
	if (getWindowSize(&E.screenrows, &E.screencols) == FAILURE) die("getWindowSize");
}


/***********************/
/*** TERMINAL SCREEN ***/
/***********************/


/* Draws (writes) to the terminal screen. */
void editorRefreshScreen(void) {
	struct abuf ab = ABUF_INIT;
	char buf[32];
	
	abAppend(&ab, "\x1b[?25l", 6);	// Hide cursor
	abAppend(&ab, "\x1b[H", 3);		// Position cursor to the start

	editorDrawRows(&ab);			// Draw each line

	// Move cursor
	snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);	// terminal commands are 1-indexed
	abAppend(&ab, buf, strlen(buf));

	abAppend(&ab, "\x1b[?25h", 6); 	// Show cursor

	// Write to terminal
	write(STDOUT_FILENO, ab.b, ab.len);
	abFree(&ab);
}

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

/************************/
/*** INPUT PROCESSING ***/
/************************/

/* Process keypress input. */
void editorProcessKeypress(void) {
	int c = editorReadKey();

	switch (c) {
		case CTRL_KEY('q'):
			editorRefreshScreen();
			exit(SUCCESS);
			break;

		case HOME_KEY:
			E.cx = 0;
			break;

		case END_KEY:
			E.cx = E.screencols - 1;
			break;

		case PAGE_UP:
		case PAGE_DOWN:
			{
				int times = E.screenrows;
				while (times--) editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
			}
			break;

		case ARROW_UP:
		case ARROW_DOWN:
		case ARROW_LEFT:
		case ARROW_RIGHT:
			editorMoveCursor(c);
			break;
	}
}

/* Read and return the character from keypress input. */
int editorReadKey(void) {
	int nread;
	char c;

	while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if (nread == FAILURE && errno != EAGAIN) die("read");
	}

	// Handling special keys
	if (c == '\x1b') {
		char seq[3];

		if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
		if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

		if (seq[0] == '[') {
			if (seq[1] >= '0' && seq[1] <= '9') {
				if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
				if (seq[2] == '~') {
					switch (seq[1]) {
						case '1': return HOME_KEY;
						case '3': return DEL_KEY;
						case '4': return END_KEY;
						case '5': return PAGE_UP;
						case '6': return PAGE_DOWN;
						case '7': return HOME_KEY;
						case '8': return END_KEY;
					}
				}
			} else {
				switch (seq[1]) {
					case 'A': return ARROW_UP;
					case 'B': return ARROW_DOWN;
					case 'C': return ARROW_RIGHT;
					case 'D': return ARROW_LEFT;
					case 'H': return HOME_KEY;
					case 'F': return END_KEY;
				}
			}
		} else if (seq[0] == 'O') {
			switch (seq[1]) {
				case 'H': return HOME_KEY;
				case 'F': return END_KEY;
			}
		}

		return '\x1b';
	} else {
		return c;
	}
}

/* Reads keypress and modifies cursor position */
void editorMoveCursor(int key) {
	switch (key) {
		case ARROW_LEFT:
			if (E.cx != 0) {
				E.cx--;
			}
			break;
		case ARROW_RIGHT:
			if (E.cx != E.screencols - 1) {
				E.cx++;
			}
 			break;
		case ARROW_UP:
			if (E.cy != 0) {
				E.cy--;
			}
			break;
		case ARROW_DOWN:
			if (E.cy != E.screenrows - 1) {
				E.cy++;
			}
			break;
	}
}
