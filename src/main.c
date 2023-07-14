#include <stdlib.h>
#include <ncurses.h>

#define CTRL(c)	((c) & 0x1f)	/* Bit flipping to represent a Ctrl key */

static void void_atexit_handler(void)
{
	endwin();	/* End curses mode */
}

static WINDOW *void_term_raw(void)
{
	WINDOW *win = initscr();	/* Start curses mode */
	atexit(void_atexit_handler);	/* Automatic exit cleanup */
	raw();				/* Disable line buffering */
	keypad(stdscr, TRUE);		/* Allow special keys reading */
	halfdelay(1);			/* Input timeout */
	noecho();			/* Turn off echoing */

	return win;
}

int main(void)
{
	int win_y = 0, win_x = 0;
	WINDOW *win = void_term_raw();
	getmaxyx(win, win_y, win_x);

	for (int i = 0, c = 0; (c = getch()) != CTRL('q'); i++) {
		if (i == win_y) {
			clear();
			i = 0;
		}

		printw("'%c', (%d)\n", c, c);
		refresh();
	}

	return EXIT_SUCCESS;
}
