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

	return V_OK;
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
	v->scr_x = 0;
	v->scr_y = 0;
	v->cur_x = 0;
	v->cur_y = 0;

	return V_OK;
}
