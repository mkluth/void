#include <ncurses.h>
#include <void.h>

int v_prcs_key(struct v_state *v)
{
	if (!v || v->v_mode != V_CMD || !v->v_scr)
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
