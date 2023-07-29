#include <ncurses.h>
#include <string.h>
#include <void.h>

static int v_vert_scroll(struct v_state *v)
{
	if (v->cur_y < v->rowoff)
		v->rowoff = v->cur_y;

	if (v->cur_y >= v->rowoff + v->scr_y)
		v->rowoff = v->cur_y - v->scr_y + 1;

	return V_OK;
}

static int v_horiz_scroll(struct v_state *v)
{
	if (v->cur_x < v->coloff)
		v->coloff = v->cur_x;

	if (v->cur_x >= v->coloff + v->scr_x)
		v->coloff = v->cur_x - v->scr_x + 1;

	return V_OK;
}

static int v_draw_y(struct v_state *v, int y)
{
	int filerow = v->rowoff + y;
	if (filerow < v->nrows) {
		/* There is a row to be displayed */
		struct v_row *row = &v->rows[filerow];
		int len = row->len - v->coloff;
		if (len < 0)
			len = 0;
		if (len > v->scr_x)
			len = v->scr_x;

		addnstr(&row->cont[v->coloff], len);

		return V_OK;
	}

	if (v->nrows == 0 && y == v->scr_y / 3) {
		/* Display welcome message */
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

		addnstr(V_WC, len);

		return V_OK;
	}

	printw("~\n");

	return V_OK;
}

static int v_draw_scr_y(struct v_state *v)
{
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
	if (!v || !v->v_stdscr || v->cur_x < 0 || v->cur_y < 0 ||
			v->rowoff < 0 || v->coloff < 0 || v->scr_y <= 0 ||
			v->scr_x <= 0)
		return V_ERR;

	v_vert_scroll(v);
	v_horiz_scroll(v);
	curs_set(2);
	v_draw_scr_y(v);
	move(v->cur_y - v->rowoff, v->cur_x - v->coloff);
	refresh();
	curs_set(1);

	return V_OK;
}
