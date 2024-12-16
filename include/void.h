#ifndef VOID_H
#define VOID_H

#include <ncurses.h>

#define V_VER		"0.0.1"
#define V_DESC		"A stupid UNIX text editor"
#define V_WC		"void/Void " V_VER " -- " V_DESC

#define V_BAR		1		/* Editor bar color pair number */
#define V_BAR_FG	COLOR_BLACK	/* Editor bar foreground color */
#define V_BAR_BG	COLOR_WHITE	/* Editor bar background color */

#define V_TABSTP	8		/* Default tabstop size */

#define V_OK		0		/* Return value success */
#define V_ERR		1		/* Return value failure */
#define V_TRUE		1		/* Boolean value for True */
#define V_FALSE		0		/* Boolean value for False */
#define V_CMD		1		/* Command mode value */
#define V_INSERT	2		/* Insert mode value */
#define CTRL(c)		((c) & 0x1f)	/* Represent a Ctrl key */

#define CUR_LEFT	104		/* Move cursor leftwards */
#define CUR_UP		107		/* Move cursor upwards */
#define CUR_DOWN	106		/* Move cursor downwards */
#define CUR_RIGHT	108		/* Move cursor rightwards */

/*
 * struct v_row - Represent a line of text to be displayed
 * orig: the original string (unrendered)
 * ren: the rendered string
 * len: the original string length (unrendered)
 * rlen: the rendered string length
 */
struct v_row {
	char *orig;
	char *ren;
	int len;
	int rlen;
};

/*
 * struct v_state - Current thread information
 * v_win: pointer to NCURSES WINDOW struct
 * rows: array of v_row structs
 * nrows: number of available v_row structs
 * scr_x: maximum value of screen x-axis
 * scr_y: maximum value of screen y-axis
 * cur_x: current cursor x-axis
 * cur_y: current cursor y-axis
 * rcur_x: current cursor x-axis (rendered)
 * rowoff: current row offset
 * coloff: current column offset
 * v_colors: colors support flag
 * filename: currently opened filename
 * unsaved: available unsaved changes
 * v_mode: current editor mode
 * v_run: current editor running status
 */
struct v_state {
	WINDOW *v_win;
	struct v_row *rows;
	int nrows;
	int scr_x;
	int scr_y;
	int cur_x;
	int cur_y;
	int rcur_x;
	int rowoff;
	int coloff;
	int v_colors;
	char *filename;
	int v_unsaved;
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
int v_init_colors(struct v_state *v);
int v_reset_term(struct v_state *v);

/* src/output.c */
int v_rfsh_scr(struct v_state *v);

/* src/fileio.c */
int v_open(struct v_state *v, char *filename);

/* src/row.c */
int v_append_row(struct v_state *v, char *s, int len);
int v_free_rows(struct v_state *v);

#endif	/* VOID_H */
