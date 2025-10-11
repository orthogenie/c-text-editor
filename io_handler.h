#ifndef IO_HANDLER_H
#define IO_HANDLER_H

#include "user_std.h"

#define CTRL_KEY(k) ((k) & 0x1f)
#define ABUF_INIT {NULL, 0}

/*** DATA ***/

/* String buffer object */
struct abuf {
	char* b;	// String buffer
	int len;	// String length
};

/*** FUNCTIONS ***/

char editorReadKey(void);
void editorProcessKeypress(void);
void editorDrawRows(struct abuf*);
void abAppend(struct abuf*, const char*, int);
void abFree(struct abuf*);

#endif
