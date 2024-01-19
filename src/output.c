#include <ncurses.h>
#include <string.h>

#include <void.h>

static int v_draw_y(struct v_state *v, int y)
{
	int filerow = v->rowoff + y;
	if (filerow < v->nrows) {
		/* There is a row to be displayed */
		struct v_row *row = &v->rows[filerow];
		int len = row->rlen - v->coloff;
		if (len < 0)
			len = 0;
		if (len > v->scr_x)
			len = v->scr_x;

		waddnstr(v->v_win, &row->ren[v->coloff], len);

		return V_OK;
	}

	if (v->nrows == 0 && y == v->scr_y / 3) {
		/* Display welcome message */
		int len = strlen(V_WC);
		if (len > v->scr_x)
			len = v->scr_x;

		int padding = (v->scr_x - len) / 2;
		if (padding) {
			wprintw(v->v_win, "~");
			padding--;
		}

		while (padding--)
			wprintw(v->v_win, " ");

		waddnstr(v->v_win, V_WC, len);

		return V_OK;
	}

	wprintw(v->v_win, "~\n");

	return V_OK;
}

static int v_draw_scr_y(struct v_state *v)
{
	for (int y = 0; y < v->scr_y; y++) {
		wmove(v->v_win, y, 0);
		v_draw_y(v, y);
		wclrtoeol(v->v_win);
	}

	return V_OK;
}

static int v_draw_bar(struct v_state *v)
{
	wmove(v->v_win, v->scr_y, 0);
	wattron(v->v_win, COLOR_PAIR(V_BAR));
	int x = 0;
	while (x < v->scr_x) {
		wprintw(v->v_win, " ");
		x++;
	}

	wattroff(v->v_win, COLOR_PAIR(V_BAR));

	return V_OK;
}

static int v_render_cur_x(struct v_row *row, int cur_x)
{
	if (!row || !row->orig || cur_x < 0)
		return -1;

	int x_pos = 0;
	for (int i = 0; i < cur_x; i++) {
		if (row->orig[i] == '\t')
			x_pos += (V_TABSTP - 1) - (x_pos % V_TABSTP);
		x_pos++;
	}

	return x_pos;
}

static int v_scroll(struct v_state *v)
{
	v->rcur_x = 0;
	if (v->cur_y < v->nrows)
		v->rcur_x = v_render_cur_x(&v->rows[v->cur_y], v->cur_x);

	if (v->cur_y < v->rowoff)
		v->rowoff = v->cur_y;

	if (v->cur_y >= v->rowoff + v->scr_y)
		v->rowoff = v->cur_y - v->scr_y + 1;

	if (v->rcur_x < v->coloff)
		v->coloff = v->rcur_x;

	if (v->rcur_x >= v->coloff + v->scr_x)
		v->coloff = v->rcur_x - v->scr_x + 1;

	return V_OK;
}

/*
 * v_rfsh_scr -- Refresh the entire editor screen
 * v: pointer to v_state struct
 *
 * Description:
 * Returns V_OK upon successful completion. Otherwise, V_ERR. Only works in
 * curses mode.
 */
int v_rfsh_scr(struct v_state *v)
{
	if (!v || !v->v_win || v->cur_x < 0 || v->cur_y < 0 ||
			v->rowoff < 0 || v->coloff < 0 || v->scr_y <= 0 ||
			v->scr_x <= 0)
		return V_ERR;

	v_scroll(v);
	curs_set(2);
	v_draw_scr_y(v);
	v_draw_bar(v);
	wmove(v->v_win, v->cur_y - v->rowoff, v->rcur_x - v->coloff);
	wrefresh(v->v_win);
	curs_set(1);

	return V_OK;
}
