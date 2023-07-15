#ifndef VOID_H
#define VOID_H

#include <ncurses.h>

#define V_OK		0		/* Return value success */
#define V_ERR		1		/* Return value failure */
#define V_CMD		1		/* Command mode value */
#define V_INSERT	2		/* Insert mode value */
#define CTRL(c)		((c) & 0x1f)	/* Represent a Ctrl key */

/*
 * struct v_state - current thread information
 * v_scr: pointer to curses WINDOW struct
 * scr_x: value of screen x-axis
 * scr_y: value of screen y-axis
 * v_mode: current editor mode
 */
struct v_state {
	WINDOW *v_scr;
	int scr_x;
	int scr_y;
	int v_mode;
};

/* src/state.c */
struct v_state *v_new_state(void);
int v_dstr_state(struct v_state *v);
int v_scr_raw(struct v_state *v);

#endif	/* VOID_H */
