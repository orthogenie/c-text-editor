#ifndef USER_DEF_H
#define USER_DEF_H

#include <termios.h>

#define KILO_VERSION "0.0.1"

#define FAILURE -1
#define SUCCESS 0

#define CTRL_KEY(k) ((k) & 0x1f)
#define ABUF_INIT {NULL, 0}


/************/
/*** DATA ***/
/************/


/* String buffer object */
struct abuf {
	char* b;	// String buffer
	int len;	// String length
};

/* Config data for the editor. */
struct editorConfig {
	int cx, cy;		// Cursor position
	int screenrows;	// Number of rows
	int screencols;	// Number of columns
	struct termios orig_termios;	// termios object
};

/* Cursor movement enum*/
enum editorKey {
	ARROW_LEFT = 1000,	// Large int outside of char range
	ARROW_RIGHT,		// 1001
	ARROW_UP,			// 1002
	ARROW_DOWN,			// 1003
	DEL_KEY,			// 1004
	HOME_KEY,			// 1005
	END_KEY,			// 1006
	PAGE_UP,			// 1007
	PAGE_DOWN			// 1008
};


/* Global editor object. */
struct editorConfig E;

#endif
