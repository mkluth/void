#include <ncurses.h>
#include <string.h>
#include <void.h>

static int v_draw_y(struct v_state *v, int y)
{
	if (!v || !v->v_stdscr || y < 0)
		return V_ERR;

	if (y == v->scr_y / 3) {
		int len = strlen(V_WC);
		if (len > v->scr_x)
			len = v->scr_x;

		int padding = (v->scr_x - len) / 2;
		if (padding) {
			printw("~");
			padding--;
		}

		while (padding--)
			printw(" ");
		printw("%s\n", V_WC);

		return V_OK;
	}

	printw("~\n");

	return V_OK;
}

static int v_draw_scr_y(struct v_state *v)
{
	if (!v || !v->v_stdscr)
		return V_ERR;

	for (int y = 0; y < v->scr_y; y++) {
		move(y, 0);
		v_draw_y(v, y);
		clrtoeol();
	}

	return V_OK;
}

/*
 * v_rfsh_scr -- Refresh the editor screen
 * v: a pointer to a v_state struct
 *
 * Description:
 * Returns V_OK upon successful completion. Otherwise, V_ERR.
 */
int v_rfsh_scr(struct v_state *v)
{
	if (!v || !v->v_stdscr)
		return V_ERR;

	curs_set(2);
	v_draw_scr_y(v);
	move(v->cur_y, v->cur_x);
	curs_set(1);
	refresh();

	return V_OK;
}
