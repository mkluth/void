#include <stddef.h>
#include <ncurses.h>
#include <stdbool.h>

#include <void.h>

static int v_cur_move(struct v_state *v, int key)
{
	struct v_row *row = (v->cur_y >= v->nrows) ? NULL : &v->rows[v->cur_y];

	switch (key) {
	case CUR_LEFT:
		/* Cursor left */
		if (v->cur_x != 0) {
			v->cur_x--;
		} else if (v->cur_y > 0) {
			v->cur_y--;
			v->cur_x = v->rows[v->cur_y].len;
		}
		break;
	case CUR_RIGHT:
		/* Cursor right */
		if (row && v->cur_x < row->len) {
			v->cur_x++;
		} else if (row && v->cur_x == row->len) {
			v->cur_y++;
			v->cur_x = 0;
		}
		break;
	case CUR_UP:
		/* Cursor up */
		if (v->cur_y != 0)
			v->cur_y--;
		break;
	case CUR_DOWN:
		/* Cursor down */
		if (v->cur_y < v->nrows)
			v->cur_y++;
		break;
	default:
		/* Other keys */
		return V_ERR;
	}

	row = (v->cur_y >= v->nrows) ? NULL : &v->rows[v->cur_y];
	int len = row ? row->len : 0;
	if (v->cur_x > len)
		v->cur_x = len;

	return V_OK;
}

static void v_page_key(struct v_state *v, int key)
{
	int times = v->scr_y;
	while (times--)
		v_cur_move(v, key == KEY_PPAGE ? CUR_UP : CUR_DOWN);
}

static int v_nav_key(struct v_state *v, int key)
{
	struct v_row *row = (v->cur_y >= v->nrows) ? NULL : &v->rows[v->cur_y];

	switch (key) {
	case KEY_LEFT:
		/* Arrow left */
		return v_cur_move(v, CUR_LEFT);
	case KEY_RIGHT:
		/* Arrow right */
		return v_cur_move(v, CUR_RIGHT);
	case KEY_UP:
		/* Arrow up */
		return v_cur_move(v, CUR_UP);
	case KEY_DOWN:
		/* Arrow down */
		return v_cur_move(v, CUR_DOWN);
	case KEY_HOME:
		/* Home key */
		v->cur_x = 0;
		return V_OK;
	case KEY_END:
		/* End key */
		if (row)
			v->cur_x = row->len;
		return V_OK;
	case KEY_PPAGE:
		/* Page Up */
		v->cur_y = v->rowoff;
		v_page_key(v, key);
		return V_OK;
	case KEY_NPAGE:
		/* Page Down */
		v->cur_y = v->rowoff + v->scr_x - 1;
		if (v->cur_y > v->nrows)
			v->cur_y = v->nrows;
		v_page_key(v, key);
		return V_OK;
	}

	return V_ERR;
}

static void v_exit(struct v_state *v)
{
	if (v->dirty)
		goto confirm_exit;

	v->v_run = false;
	return;

confirm_exit:
	v_set_stats_msg(v, "WARNING: Unsaved changes. Press again to confirm.");
	v_rfsh_scr(v);
	if (wgetch(v->v_win) != CTRL('q')) {
		v_set_stats_msg(v, "");
		return;
	}

	v->v_run = false;
}

static int v_cmd_input(struct v_state *v, int key)
{
	if (v_cur_move(v, key) == V_OK || v_nav_key(v, key) == V_OK)
		return V_OK;

	switch (key) {
	case CTRL('q'):
		/* Ctrl-Q: Exit the editor */
		v_exit(v);
		return V_OK;
	case 'i':
	case 'I':
		/* i or I: Switch into Insert Mode */
		v->v_mode = V_INSERT;
		v_set_stats_msg(v, "-- INSERT --");
		return V_OK;
	case 'x':
	case 'X':
	case KEY_DC:
		/*
		 * x or X or DELETE:
		 * Delete the character at the cursor's position
		 */
		v_row_del_char(&v->rows[v->cur_y], v->cur_x);
		v->dirty = true;
		return V_OK;
	case CTRL('s'):
		/* Ctrl-S: Write out to a file */
		v_save(v);
		return V_OK;
	case CTRL('l'):
		/* Ctrl-L: Force refresh the editor screen */
		v_rfsh_scr(v);
		return V_OK;
	}

	return V_ERR;
}

static int v_insert_input(struct v_state *v, int key)
{
	if (v_nav_key(v, key) == V_OK)
		return V_OK;

	switch (key) {
	case CTRL('['):
		/* ESC or Ctrl-[: Switch into Command Mode */
		v->v_mode = V_CMD;
		v_set_stats_msg(v, "");
		return V_OK;
	case '\r':
	case '\n':
		/* TODO */
		return V_OK;
	case KEY_BACKSPACE:
	case CTRL('h'):
		/* BACKSPACE or Ctrl-H: Delete a character on the leftside */
		v_backspace(v);
		return V_OK;
	case KEY_DC:
		/* DELETE: Delete the character at the cursor's position */
		v_row_del_char(&v->rows[v->cur_y], v->cur_x);
		v->dirty = true;
		return V_OK;
	case CTRL('l'):
		/* Ctrl-L: Force refresh the editor screen */
		v_rfsh_scr(v);
		return V_OK;
	}

	if (v_insert(v, key) != V_ERR)
		return V_OK;

	return V_ERR;
}

/**
 * v_prcs_key - read a key and process it for the specified v_state
 * v: Pointer to the targeted v_state struct.
 *
 * Read a key and process it for the specified v_state. The input processing is
 * done according to the specified v_state current editor mode. Please take note
 * that the v_state curses window, v->v_win is required as we will only read the
 * key on that v_state specific curses window rather than on the stdscr one.
 *
 * Returns V_OK on success, V_ERR otherwise.
 */
int v_prcs_key(struct v_state *v)
{
	if (!v || !v->v_win)
		return V_ERR;

	int stats = V_OK;
	int key = wgetch(v->v_win);

	switch (v->v_mode) {
	case V_CMD:
		/* Command Mode */
		stats = v_cmd_input(v, key);
		break;
	case V_INSERT:
		/* Insert Mode */
		stats = v_insert_input(v, key);
		break;
	default:
		/* Unknown Mode */
		stats = V_ERR;
		break;
	}

	return stats;
}
