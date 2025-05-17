/*
 * void.h - Core editor structs, macros and operation routines declarations
 *
 * Current development and maintenance by:
 * 	Copyright (c) 2025-Present Luth <https://github.com/mkluth>
 *
 * This file is a part of the void text editor.
 * It is licensed under MIT License. See the LICENSE file for details.
 */

#ifndef VOID_H
#define VOID_H

#include <stdbool.h>
#include <stddef.h>

#define V_VER		"0.0.1"
#define V_DESC		"Built like Vim and GNU nano, but worse."
#define V_WELCOME_MSG	"void " V_VER " -- " V_DESC
#define V_AUTHOR	"by Luth <https://github.com/mkluth>"

#define V_STATS_MSG_BUF		100	/* Status message buffer */
#define V_DEFAULT_BUF_SZ	128	/* Default prompt buffer size */
#define V_STATS_LEFT_MAX	128	/* Default left status bar length */
#define V_STATS_RIGHT_MAX	32	/* Default right status bar length */
#define V_WELCOME_MSG_BUF	100	/* Default welcome message length */

#define V_BAR		1		/* Editor bar color pair number */
#define V_BAR_FG	COLOR_BLACK	/* Editor bar foreground color */
#define V_BAR_BG	COLOR_WHITE	/* Editor bar background color */

#define V_TABSTP	8		/* Default tabstop size */
#define V_FILE_MODE	0644		/* Default text files permission */

#define V_OK		0		/* Return value success */
#define V_ERR		-1		/* Return value failure */
#define V_CMD		1		/* Command mode value */
#define V_INSERT	2		/* Insert mode value */
#define CTRL(c)		((c) & 0x1f)	/* Represents a Ctrl key */
#define V_KEY_ESC	27		/* Represents an ESC key */
#define V_KEY_NL	10		/* Represents a '\n' key */
#define V_KEY_RET	13		/* Represents a '\r' key */
#define V_KEY_BKSP	127		/* Represents a BACKSPACE key */

/**
 * struct v_row - represent a line of text to be displayed
 * orig: The original string (unrendered).
 * ren: The rendered string.
 * len: The original string length (unrendered).
 * rlen: The rendered string length.
 */
struct v_row {
	char *orig;
	char *ren;
	int len;
	int rlen;
};

/**
 * struct v_state - current thread information
 * rows: Array of v_row structs.
 * nrows: Number of available v_row structs.
 * scr_x: Maximum value of screen x-axis.
 * scr_y: Maximum value of screen y-axis.
 * cur_x: Current cursor x-axis.
 * cur_y: Current cursor y-axis.
 * rcur_x: Current cursor x-axis (rendered).
 * rowoff: Current row offset.
 * coloff: Current column offset.
 * colors: Colors support flag.
 * filename: Currently opened filename.
 * stats_msg: Status message string (view V_STATS_MSG_BUF macro).
 * dirty: Available unsaved changes.
 * mode: Current editor mode.
 * run: Current editor running status.
 */
struct v_state {
	struct v_row *rows;
	int nrows;
	int scr_x;
	int scr_y;
	int cur_x;
	int cur_y;
	int rcur_x;
	int rowoff;
	int coloff;
	bool colors;
	char *filename;
	char stats_msg[V_STATS_MSG_BUF];
	bool dirty;
	int mode;
	bool run;
};

/**
 * struct v_key - represent the editor single-key keybinding
 * key: The registered ASCII value for the keybinding.
 * func: Pointer to the keybinding function.
 */
struct v_key {
	int key;
	int (*func)(struct v_state *v);
};

/* src/state.c */
struct v_state *v_new_state(void);
int v_dstr_state(struct v_state *v);

/* src/cursor.c */
int v_cur_left(struct v_state *v);
int v_cur_right(struct v_state *v);
int v_cur_up(struct v_state *v);
int v_cur_down(struct v_state *v);
int v_cur_bol(struct v_state *v);
int v_cur_eol(struct v_state *v);
int v_ppage(struct v_state *v);
int v_npage(struct v_state *v);
int v_bottom_pg(struct v_state *v);
int v_top_pg(struct v_state *v);

/* src/input.c */
int v_prcs_key(struct v_state *v);
char *v_prompt(struct v_state *v, char *s);

/* src/term.c */
int v_init_term(struct v_state *v);
int v_init_colors(struct v_state *v);
int v_reset_term(struct v_state *v);

/* src/output.c */
int v_set_stats_msg(struct v_state *v, const char *fmt, ...);
int v_rfsh_scr(struct v_state *v);

/* src/fileio.c */
int v_open(struct v_state *v, char *filename);
int v_save(struct v_state *v);

/* src/editor.c */
int v_insert(struct v_state *v, int c);
int v_insert_nl(struct v_state *v);
int v_backspace(struct v_state *v);
int v_right_backspace(struct v_state *v);

/* src/row.c */
int v_render_row(struct v_row *row);
int v_insert_row(struct v_state *v, int y, char *s, size_t len);
int v_del_row(struct v_state *v, int y);
int v_free_rows(struct v_state *v);
int v_row_insert_char(struct v_row *row, int at, int c);
int v_row_append_str(struct v_row *row, char *s, size_t len);
int v_row_del_char(struct v_row *row, int x);

#endif	/* VOID_H */
