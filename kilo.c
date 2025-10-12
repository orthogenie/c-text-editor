#include "editor.h"

/* KILO TEXT EDITOR */
int main (void) {
	enableRawMode();
	initEditor();

	while (1) {
		editorRefreshScreen();
		editorProcessKeypress();
	}

	return SUCCESS;
}
