#ifndef UTIL_H
#define UTIL_H

#include "user_def.h"

/*****************/
/*** FUNCTIONS ***/
/*****************/

void abAppend(struct abuf*, const char*, int);
void abFree(struct abuf*);
void die(const char*);
void disableRawMode(void);
void enableRawMode(void);
int getWindowSize(int*, int*);
int getCursorPosition(int*, int*);

#endif
