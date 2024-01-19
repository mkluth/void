#include <ncurses.h>

#include <void.h>

/*
 * v_init_term - Initialize the terminal into curses mode
 * v: pointer to v_state struct
 *
 * Description:
 * Returns V_OK if successful, otherwise V_ERR. Don't forget to call
 * v_reset_term() before exiting the program.
 */
int v_init_term(struct v_state *v)
{
	if (!v || v->v_win)
		return V_ERR;

	v->v_win = initscr();
	raw();
	keypad(v->v_win, TRUE);
	noecho();
	halfdelay(1);

	getmaxyx(v->v_win, v->scr_y, v->scr_x);
	v->scr_y -= 1;

	return V_OK;
}

/*
 * v_init_colors - Initialize the terminal colors support
 * v: pointer to v_state struct
 *
 * Description:
 * Returns V_OK if the terminal does support colors manipulation. Otherwise,
 * V_ERR shall be returns. You can only call this function once v_init_term()
 * is called previously.
 */
int v_init_colors(struct v_state *v)
{
	if (!v->v_win || !has_colors())
		goto error;

	v->v_colors = V_TRUE;
	start_color();
	init_pair(V_BAR, V_BAR_FG, V_BAR_BG);

	return V_OK;

error:
	v->v_colors = V_FALSE;
	return V_ERR;
}

/*
 * v_reset_term - Reset the terminal back into cooked mode
 * v: pointer to v_state struct
 *
 * Description:
 * Returns V_OK if successful, else V_ERR. This function should be called
 * before exiting the program if v_init_term() is called previously.
 */
int v_reset_term(struct v_state *v)
{
	if (!v || !v->v_win)
		return V_ERR;

	endwin();
	v->v_win = NULL;
	v->v_colors = V_FALSE;
	v->scr_x = 0;
	v->scr_y = 0;
	v->cur_x = 0;
	v->cur_y = 0;

	return V_OK;
}
