#include "editor.h"
#include "io_handler.h"
#include "user_def.h"

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
