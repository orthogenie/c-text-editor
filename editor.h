#ifndef EDITOR_H
#define EDITOR_H

#include <termios.h>
#include "user_def.h"

/************/
/*** DATA ***/
/************/

/* Config data for the editor. */
struct editorConfig {
	int cx, cy;		// Cursor position
	int screenrows;	// Number of rows
	int screencols;	// Number of columns
	struct termios orig_termios;	// termios object
};

/* Global editor object. */
struct editorConfig E;


/*****************/
/*** FUNCTIONS ***/
/*****************/

void die(const char*);
void disableRawMode(void);
void enableRawMode(void);
void editorRefreshScreen(void);
int getWindowSize(int*, int*);
int getCursorPosition(int*, int*);
void initEditor(void);

#endif
