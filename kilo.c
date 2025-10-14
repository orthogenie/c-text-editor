#include "editor.h"

/* KILO TEXT EDITOR */
int main (int argc, char** argv) {
	enableRawMode();
	initEditor();
	if (argc >= 2) {
		editorOpen(argv[1]);
	}

	while (1) {
		editorRefreshScreen();
		editorProcessKeypress();
	}

	return SUCCESS;
}
