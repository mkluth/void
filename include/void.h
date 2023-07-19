#ifndef VOID_H
#define VOID_H

#include <ncurses.h>

#define V_OK		0		/* Return value success */
#define V_ERR		1		/* Return value failure */
#define V_TRUE		1		/* Boolean value for True */
#define V_FALSE		0		/* Boolean value for False */
#define V_CMD		1		/* Command mode value */
#define V_INSERT	2		/* Insert mode value */
#define CTRL(c)		((c) & 0x1f)	/* Represent a Ctrl key */

/*
 * struct v_state - current thread information
 * v_stdscr: pointer to NCURSES stdscr WINDOW struct
 * scr_x: value of screen x-axis
 * scr_y: value of screen y-axis
 * v_mode: current editor mode
 * v_run: editor current running status
 */
struct v_state {
	WINDOW *v_stdscr;
	int scr_x;
	int scr_y;
	int v_mode;
	int v_run;
};

/* src/state.c */
struct v_state *v_new_state(void);
int v_dstr_state(struct v_state *v);

/* src/input.c */
int v_prcs_key(struct v_state *v);

/* src/term.c */
int v_init_term(struct v_state *v);
int v_reset_term(struct v_state *v);

#endif	/* VOID_H */
