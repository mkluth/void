#include <ncurses.h>
#include <void.h>

static void v_pg_up(struct v_state *v)
{
	int times = v->scr_y;
	while (times-- && v->cur_y != 0)
		v->cur_y--;
}

static void v_pg_down(struct v_state *v)
{
	int times = v->scr_y;
	while (times-- && v->cur_y != v->scr_y - 1)
		v->cur_y++;
}

static int v_mv_cur(struct v_state *v, int key)
{
	if (!v || !v->v_stdscr || v->cur_x < 0 || v->cur_y < 0)
		return V_ERR;

	switch (key) {
	case CUR_LEFT:
	case KEY_LEFT:
		/* Move cursor leftwards */
		if (v->cur_x != 0)
			v->cur_x--;
		return V_OK;
	case CUR_RIGHT:
	case KEY_RIGHT:
		/* Move cursor rightwards */
		if (v->cur_x != v->scr_x - 1)
			v->cur_x++;
		return V_OK;
	case CUR_UP:
	case KEY_UP:
		/* Move cursor upwards */
		if (v->cur_y != 0)
			v->cur_y--;
		return V_OK;
	case CUR_DOWN:
	case KEY_DOWN:
		/* Move cursor downwards */
		if (v->cur_y != v->scr_y - 1)
			v->cur_y++;
		return V_OK;
	case KEY_HOME:
		/* Move cursor via Home key */
		v->cur_x = 0;
		return V_OK;
	case KEY_END:
		/* Move cursor via End key */
		v->cur_x = v->scr_x - 1;
		return V_OK;
	case KEY_PPAGE:
		/* Move cursor via Page Up */
		v_pg_up(v);
		return V_OK;
	case KEY_NPAGE:
		/* Move cursor via Page Down */
		v_pg_down(v);
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
	if (!v || v->v_mode != V_CMD || !v->v_stdscr)
		return V_ERR;

	int key = getch();

	if (v_mv_cur(v, key) == V_OK)
		return V_OK;

	switch (key) {
	case CTRL('q'):
		/* Ctrl-Q: Exit the editor */
		v->v_run = V_FALSE;
		break;
	default:
		/* None of the above */
		return V_ERR;
	}

	return V_OK;
}
