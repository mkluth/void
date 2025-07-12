/*
 * term.c - Terminal window manipulation routines
 *
 * This file provides routines for initializing the terminal window into curses
 * mode, setting up curses colors support and resetting the terminal window
 * state back into cooked mode. All of the routines available here heavily
 * rely on ncurses.
 *
 * Current development and maintenance by:
 * 	Copyright (c) 2025-Present Luth <https://github.com/mkluth>
 *
 * This file is a part of the void text editor.
 * It is licensed under MIT License. See the LICENSE file for details.
 */

#include <signal.h>
#include <ncurses.h>
#include <stdbool.h>

#include <void.h>

volatile sig_atomic_t v_winch = 0;

static void v_handle_winch(int sig)
{
	(void)sig;	/* Silence compiler warning */
	v_winch = 1;
}

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
	if (!v)
		return V_ERR;

	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();
	set_escdelay(0);

	getmaxyx(stdscr, v->scr_y, v->scr_x);
	v->scr_y -= 2;

	signal(SIGWINCH, v_handle_winch);

	return V_OK;
}

/**
 * v_init_colors - initialize colors support for the specified v_state
 * v: Pointer to the targeted v_state struct.
 *
 * Initialize colors support for the specified v_state. You should only call
 * this function once v_init_term() is called previously. This function will
 * sets the v->colors flag to true if the terminal does support colors
 * manipulation and the editor NCURSES color pairs will be defined after.
 *
 * Returns V_OK if the terminal supports colors manipulation, V_ERR otherwise.
 */
int v_init_colors(struct v_state *v)
{
	if (!has_colors())
		goto error;

	v->colors = true;
	start_color();
	init_pair(V_BAR, V_BAR_FG, V_BAR_BG);

	return V_OK;

error:
	v->colors = false;
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
	if (!v)
		return V_ERR;

	endwin();
	v->colors = false;
	v->scr_x = 0;
	v->scr_y = 0;
	v->cur_x = 0;
	v->cur_y = 0;

	return V_OK;
}
