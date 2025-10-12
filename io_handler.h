#ifndef IO_HANDLER_H
#define IO_HANDLER_H

#include "user_def.h"

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


/*****************/
/*** FUNCTIONS ***/
/*****************/

void abAppend(struct abuf*, const char*, int);
void abFree(struct abuf*);
void editorMoveCursor(int);
int editorReadKey(void);
void editorProcessKeypress(void);


#endif
