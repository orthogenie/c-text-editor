#ifndef EDITOR_H
#define EDITOR_H

#include "user_def.h"
#include "util.h"


/*****************/
/*** FUNCTIONS ***/
/*****************/


void editorDrawRows(struct abuf*);
void editorMoveCursor(int);
void editorProcessKeypress(void);
int editorReadKey(void);
void editorRefreshScreen(void);
void initEditor(void);

#endif
