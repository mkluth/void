#include <stddef.h>
#include <ncurses.h>

#include <void.h>

static int v_cur_move(struct v_state *v, int key)
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

static void v_page_key(struct v_state *v, int key)
{
	int times = v->scr_y;
	while (times--)
		v_cur_move(v, key == KEY_PPAGE ? CUR_UP : CUR_DOWN);
}

static int v_navigate_key(struct v_state *v, int key)
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

static int v_cmd_mode_input(struct v_state *v, int key)
{
	if (v_cur_move(v, key) == V_OK || v_navigate_key(v, key) == V_OK)
		return V_OK;

	switch (key) {
	case CTRL('q'):
		/* Ctrl-Q: Exit the editor */
		v->v_run = V_FALSE;
		return V_OK;
	case 'i':
	case 'I':
		/* i or I: Switch into Insert Mode */
		v->v_mode = V_INSERT;
		v_set_stats_msg(v, "-- INSERT --");
		return V_OK;
	}

	return V_ERR;
}

static int v_insert_mode_input(struct v_state *v, int key)
{
	if (v_navigate_key(v, key) == V_OK)
		return V_OK;

	switch (key) {
	case CTRL('['):
	case CTRL('\n'):
		/*
		 * ESC or Ctrl-[ or Ctrl-Enter: Switch into Command Mode
		 *
		 * Unfortunately, there will be a slight delay of 1 second if
		 * the ESC key is pressed. This is due to the fact that I
		 * enabled the keypad() function for the editor when the
		 * terminal is being initialized into curses mode and I don't
		 * have any plan of changing it in the future. Like, I use
		 * keypad keys a lot, so don't question me here.
		 */
		v->v_mode = V_CMD;
		v_set_stats_msg(v, "");
		return V_OK;
	case '\r':
		/* TODO */
		return V_OK;
	case KEY_BACKSPACE:
	case CTRL('h'):
	case KEY_DL:
	case KEY_DC:
		/* TODO */
		return V_OK;
	case CTRL('l'):
		/* Will not be handled */
		return V_OK;
	}

	if (v_insert_char(v, key) != -1)
		return V_OK;

	return V_ERR;
}

/*
 * v_prcs_key - Read a key from the user and process it
 * v: pointer to v_state struct
 *
 * Description:
 * Upon successful completion, V_OK shall be returned. Otherwise, V_ERR shall
 * be returned instead. Do note that this function only works in curses mode
 * and in the editor command mode (currently).
 */
int v_prcs_key(struct v_state *v)
{
	if (!v || !v->v_win || v->cur_x < 0 || v->cur_y < 0 || v->scr_x <= 0 ||
			v->scr_y <= 0)
		return V_ERR;

	int stats = V_OK;
	int key = getch();

	if (v->v_mode == V_CMD)
		stats = v_cmd_mode_input(v, key);
	else
		stats = v_insert_mode_input(v, key);

	return stats;
}
