#include <ncurses.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include <void.h>

static void v_draw_y(struct v_state *v, int y)
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

		return;
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

		return;
	}

	wprintw(v->v_win, "~\n");
}

static void v_draw_scr_y(struct v_state *v)
{
	for (int y = 0; y < v->scr_y; y++) {
		wmove(v->v_win, y, 0);
		v_draw_y(v, y);
		wclrtoeol(v->v_win);
	}
}

static int v_draw_bar(struct v_state *v)
{
	wmove(v->v_win, v->scr_y, 0);
	char txt[v->scr_x];
	int len = snprintf(txt, sizeof(txt), "%s %s %d,%d",
			   v->filename ? v->filename : "[No Name]",
			   v->dirty ? "[+]" : "", v->cur_y + 1,
			   v->rcur_x + 1);
	if (len < 0)
		return V_ERR;

	if (len > v->scr_x)
		len = v->scr_x;

	if (v->v_colors)
		wattron(v->v_win, COLOR_PAIR(V_BAR));

	waddnstr(v->v_win, txt, len);
	while (len < v->scr_x) {
		wprintw(v->v_win, " ");
		len++;
	}

	if (v->v_colors)
		wattroff(v->v_win, A_BOLD | COLOR_PAIR(V_BAR));

	wprintw(v->v_win, "\r\n");

	return V_OK;
}

static int v_render_cur_x(struct v_row *row, int cur_x)
{
	if (!row || !row->orig || cur_x < 0)
		return V_ERR;

	int x_pos = 0;
	for (int i = 0; i < cur_x; i++) {
		if (row->orig[i] == '\t')
			x_pos += (V_TABSTP - 1) - (x_pos % V_TABSTP);
		x_pos++;
	}

	return x_pos;
}

static void v_scroll(struct v_state *v)
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
}

/*
 * v_set_stats_msg - Set the editor status message
 * v: pointer to v_state struct
 * fmt: formatted string
 *
 * Description:
 * Returns the length of the formatted string if success, otherwise V_ERR shall
 * be returned instead. This function is variadic. It can takes any number of
 * arguments just like printf() do. This function will saves the formatted
 * string into v->stats_msg. The maximum buffer of v->stats_msg depending on
 * the value of V_STATS_MSG_BUF macro. Do note that a status message will not
 * permanently displayed, it will get erased once the timestamp of the message
 * exceeded the value of V_STATS_MSG_TIMEOUT seconds. The higher the value
 * of V_STATS_MSG_TIMEOUT is, the longer the time the message will be displayed.
 */
int v_set_stats_msg(struct v_state *v, const char *fmt, ...)
{
	if (!v)
		return V_ERR;

	va_list ap;
	va_start(ap, fmt);
	int len = vsnprintf(v->stats_msg, sizeof(v->stats_msg), fmt, ap);
	if (len < 0)
		return V_ERR;

	va_end(ap);
	v->stats_msg_time = time(NULL);

	return len;
}

static void v_draw_msg_bar(struct v_state *v)
{
	wclrtoeol(v->v_win);
	int msg_len =  strlen(v->stats_msg);

	if (msg_len > v->scr_x)
		msg_len = v->scr_x;

	if (msg_len)
		waddnstr(v->v_win, v->stats_msg, msg_len);
}

/*
 * v_rfsh_scr - Refresh the entire editor screen
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
	v_draw_msg_bar(v);
	wmove(v->v_win, v->cur_y - v->rowoff, v->rcur_x - v->coloff);
	wrefresh(v->v_win);
	curs_set(1);

	return V_OK;
}
