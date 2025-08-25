/*
 * output.c - Output-related routines
 *
 * This file contains routines responsible for the editor’s screen output and
 * rendering, including the stupid one line welcome message, screen refresh
 * logic, status bar, message bar, scrolling, and cursor positioning. These
 * routines rely heavily on ncurses.
 *
 * Parts of this file are based on the kilo text editor by Salvatore Sanfilippo
 * and Paige Ruten (snaptoken)'s Build Your Own Text Editor booklet:
 *	Copyright (c) 2016 Salvatore Sanfilippo <antirez@gmail.com>
 *	Copyright (c) 2017 Paige Ruten <paige.ruten@gmail.com>
 *
 * Current development and maintenance by:
 * 	Copyright (c) 2025-Present Luth <https://github.com/mkluth>
 *
 * This file is a part of the void text editor.
 * It is licensed under MIT License. See the LICENSE file for details.
 */

#include <ncurses.h>
#include <string.h>
#include <stdarg.h>

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

		addnstr(&row->ren[v->coloff], len);

		return;
	}

	if (v->nrows == 0 && (y == v->scr_y / 3 || y == v->scr_y / 3 + 1)) {
		/* Display the stupid two lines welcome message */
		char msg[V_WELCOME_MSG_BUF];
		int len = 0;

		if (y == v->scr_y / 3)
			len = snprintf(msg, sizeof(msg), V_WELCOME_MSG);
		else if (y == v->scr_y / 3 + 1)
			len = snprintf(msg, sizeof(msg), V_AUTHOR);

		if (len < 0)
			goto tildes;

		if (len > v->scr_x)
			len = v->scr_x;

		int padding = (v->scr_x - len) / 2;
		if (padding) {
			printw("~");
			padding--;
		}

		while (padding--)
			printw(" ");

		addnstr(msg, len);

		return;
	}

tildes:
	printw("~\n");
}

static int v_draw_bar(struct v_state *v)
{
	move(v->scr_y, 0);
	char left[V_STATS_LEFT_MAX], right[V_STATS_RIGHT_MAX];
	int left_len = snprintf(left, sizeof(left), "%.20s %s",
			       v->filename ? v->filename : "[No Name]",
			       v->dirty ? "[+]" : "");

	int right_len = snprintf(right, sizeof(right), "%d,%d   ",
				v->cur_y + 1,
				v->rcur_x + 1);

	if (left_len < 0 || right_len < 0)
		return V_ERR;

	if (left_len + right_len > v->scr_x) {
		left_len = v->scr_x - right_len;
		if (left_len < 0)
			left_len = 0;
	}

	if (v->colors)
		attron(COLOR_PAIR(V_BAR));

	addnstr(left, left_len);
	for (int i = left_len; i < v->scr_x - right_len; i++)
		printw(" ");

	addnstr(right, right_len);

	if (v->colors)
		attroff(A_BOLD | COLOR_PAIR(V_BAR));

	printw("\n");

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

/**
 * v_set_stats_msg - set the editor status message for the specified v_state
 * v: Pointer to the targeted v_state struct.
 * fmt: The formatted string.
 *
 * Set the editor status message for the specified v_state. This function is
 * variadic. It can takes any number of arguments just like printf() do. This
 * function will saves the formatted string into v->stats_msg. The maximum
 * buffer of v->stats_msg depends on the value of V_STATS_MSG_BUF macro.
 *
 * Returns the length of the status message on success, V_ERR otherwise.
 */
int v_set_stats_msg(struct v_state *v, const char *fmt, ...)
{
	if (!v)
		return V_ERR;

	va_list ap;
	va_start(ap, fmt);
	int len = vsnprintf(v->stats_msg, sizeof(v->stats_msg), fmt, ap);
	va_end(ap);

	if (len < 0)
		return V_ERR;

	return len;
}

static void v_draw_msg_bar(struct v_state *v)
{
	move(v->scr_y + 1, 0);
	clrtoeol();

	int msg_len =  strlen(v->stats_msg);
	if (msg_len > v->scr_x)
		msg_len = v->scr_x;
	if (msg_len)
		addnstr(v->stats_msg, msg_len);
}

/**
 * v_rfsh_scr - refresh the editor screen using the specified v_state
 * v: Pointer to the targeted v_state struct.
 *
 * Refresh the editor screen using the specified v_state. Please take note that
 * this function only works in curses mode and also responsive to SIGWINCH
 * signal.
 *
 * Returns V_OK on success, V_ERR otherwise.
 */
int v_rfsh_scr(struct v_state *v)
{
	if (!v)
		return V_ERR;

	if (v_winch) {
		endwin();
		refresh();
		clear();
		v_winch = 0;
	}

	getmaxyx(stdscr, v->scr_y, v->scr_x);
	v->scr_y -= 2;

	v_scroll(v);
	curs_set(0);

	for (int y = 0; y < v->scr_y; y++) {
		move(y, 0);
		v_draw_y(v, y);
		clrtoeol();
	}

	v_draw_bar(v);
	v_draw_msg_bar(v);
	move(v->cur_y - v->rowoff, v->rcur_x - v->coloff);
	refresh();
	curs_set(1);

	return V_OK;
}
