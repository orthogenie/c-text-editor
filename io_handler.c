#include "io_handler.h"
#include "editor.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*************/
/*** INPUT ***/
/*************/

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

/* Read and return the character from keypress input. */
char editorReadKey(void) {
	int nread;
	char c;

	while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if (nread == FAILURE && errno != EAGAIN) die("read");
	}
	return c;
}
/**************/
/*** OUTPUT ***/
/**************/

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
