#include <stddef.h>
#include <ncurses.h>
#include <void.h>

static int v_cur(struct v_state *v, int key)
{
	struct v_row *row = (v->cur_y >= v->nrows) ? NULL : &v->rows[v->cur_y];

	switch (key) {
	case CUR_LEFT:
		/* Cursor left */
		if (v->cur_x != 0)
			v->cur_x--;
		break;
	case CUR_RIGHT:
		/* Cursor right */
		if (row && v->cur_x < row->len)
			v->cur_x++;
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
		return V_ERR;
	}

	row = (v->cur_y >= v->nrows) ? NULL : &v->rows[v->cur_y];
	int len = row ? row->len : 0;
	if (v->cur_x > len)
		v->cur_x = len;

	return V_OK;
}

static void v_pg(struct v_state *v, int key)
{
	int times = v->scr_y;
	while (times--)
		v_cur(v, key == KEY_PPAGE ? CUR_UP : CUR_DOWN);
}

static int v_nav(struct v_state *v, int key)
{
	struct v_row *row = (v->cur_y >= v->nrows) ? NULL : &v->rows[v->cur_y];

	switch (key) {
	case KEY_LEFT:
		/* Arrow left */
		return v_cur(v, CUR_LEFT);
	case KEY_RIGHT:
		/* Arrow right */
		return v_cur(v, CUR_RIGHT);
	case KEY_UP:
		/* Arrow up */
		return v_cur(v, CUR_UP);
	case KEY_DOWN:
		/* Arrow down */
		return v_cur(v, CUR_DOWN);
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
		v_pg(v, key);
		return V_OK;
	case KEY_NPAGE:
		/* Page Down */
		v->cur_y = v->rowoff + v->scr_x - 1;
		if (v->cur_y > v->nrows)
			v->cur_y = v->nrows;
		v_pg(v, key);
		return V_OK;
	}

	return V_ERR;
}

/*
 * v_prcs_key - Read a key from the user and process it
 * v: pointer to a v_state struct
 *
 * Description:
 * Upon successful completion, V_OK shall be returned. Otherwise, V_ERR shall
 * be returned instead. Do note that this function only works in curses mode
 * and in the editor command mode.
 */
int v_prcs_key(struct v_state *v)
{
	if (!v || v->v_mode != V_CMD || !v->v_stdscr || v->cur_x < 0 ||
			v->cur_y < 0 || v->scr_x <= 0 || v->scr_y <= 0)
		return V_ERR;

	int key = getch();

	if (v_cur(v, key) == V_OK || v_nav(v, key) == V_OK)
		return V_OK;

	switch (key) {
	case CTRL('q'):
		/* Ctrl-Q: Exit the editor */
		v->v_run = V_FALSE;
		return V_OK;
	}

	return V_ERR;
}
