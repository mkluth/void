#include <ncurses.h>
#include <stdbool.h>

#include <void.h>

/**
 * v_init_term - initialize the specifed v_state terminal into curses mode
 * v: Pointer to the targeted v_state struct.
 *
 * Initialize the specified v_state terminal into curses mode. Don't forget to
 * call v_reset_term() before exiting the program.
 *
 * Returns V_OK on success, otherwise V_ERR.
 */
int v_init_term(struct v_state *v)
{
	if (!v || v->v_win)
		return V_ERR;

	v->v_win = initscr();
	raw();
	keypad(v->v_win, TRUE);
	noecho();
	set_escdelay(0);

	getmaxyx(v->v_win, v->scr_y, v->scr_x);
	v->scr_y -= 2;

	return V_OK;
}

/**
 * v_init_colors - initialize colors support for the specified v_state
 * v: Pointer to the targeted v_state struct.
 *
 * Initialize colors support for the specified v_state. You can only call this
 * function once v_init_term() is called previously. This function will sets the
 * v->v_colors flag to true if the terminal does support colors manipulation and
 * the editor NCURSES color pairs will be defined after.
 *
 * Returns V_OK if the terminal supports colors manipulation, V_ERR otherwise.
 */
int v_init_colors(struct v_state *v)
{
	if (!v->v_win || !has_colors())
		goto error;

	v->v_colors = true;
	start_color();
	init_pair(V_BAR, V_BAR_FG, V_BAR_BG);

	return V_OK;

error:
	v->v_colors = false;
	return V_ERR;
}

/**
 * v_reset_term - reset the specified v_state terminal back into cooked mode
 * v: Pointer to the targeted v_state struct.
 *
 * Reset the specified v_state terminal back into cooked mode. This function
 * should be called before exiting the program if v_init_term() is called
 * previously. All of the attributes related to the curses mode in the specified
 * v_state struct will erased.
 *
 * Returns V_OK on success, V_ERR otherwise.
 */
int v_reset_term(struct v_state *v)
{
	if (!v || !v->v_win)
		return V_ERR;

	endwin();
	v->v_win = NULL;
	v->v_colors = false;
	v->scr_x = 0;
	v->scr_y = 0;
	v->cur_x = 0;
	v->cur_y = 0;

	return V_OK;
}
