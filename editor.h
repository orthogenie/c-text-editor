#ifndef EDITOR_H
#define EDITOR_H

#include "user_def.h"
#include "util.h"

#include <stdlib.h>


/*****************/
/*** FUNCTIONS ***/
/*****************/

void editorAppendRow(char*, size_t);
void editorDrawRows(struct abuf*);
void editorMoveCursor(int);
void editorOpen(char*);
void editorProcessKeypress(void);
int editorReadKey(void);
void editorRefreshScreen(void);
void initEditor(void);

#endif
