#include <ncurses.h>
#include <void.h>

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

	switch (getch()) {
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
