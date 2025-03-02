#ifndef VOID_H
#define VOID_H

#include <ncurses.h>
#include <stdbool.h>

#define V_VER		"0.0.1"
#define V_DESC		"A stupid UNIX text editor"
#define V_WC		"void/Void " V_VER " -- " V_DESC

#define V_STATS_MSG_BUF		100	/* Status message buffer */

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
#define V_KEY_NEWLINE	10		/* Represents '\n' key */
#define V_KEY_RETURN	13		/* Represents '\r' key */

#define CUR_LEFT	104		/* Move cursor leftwards */
#define CUR_UP		107		/* Move cursor upwards */
#define CUR_DOWN	106		/* Move cursor downwards */
#define CUR_RIGHT	108		/* Move cursor rightwards */

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
 * v_win: Pointer to NCURSES WINDOW struct.
 * rows: Array of v_row structs.
 * nrows: Number of available v_row structs.
 * scr_x: Maximum value of screen x-axis.
 * scr_y: Maximum value of screen y-axis.
 * cur_x: Current cursor x-axis.
 * cur_y: Current cursor y-axis.
 * rcur_x: Current cursor x-axis (rendered).
 * rowoff: Current row offset.
 * coloff: Current column offset.
 * v_colors: Colors support flag.
 * filename: Currently opened filename.
 * stats_msg: Status message string (view V_STATS_MSG_BUF macro).
 * dirty: Available unsaved changes.
 * v_mode: Current editor mode.
 * v_run: Current editor running status.
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
	bool v_colors;
	char *filename;
	char stats_msg[V_STATS_MSG_BUF];
	bool dirty;
	int v_mode;
	bool v_run;
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
int v_set_stats_msg(struct v_state *v, const char *fmt, ...);
int v_rfsh_scr(struct v_state *v);

/* src/fileio.c */
int v_open(struct v_state *v, char *filename);
int v_save(struct v_state *v);

/* src/editor.c */
int v_insert(struct v_state *v, int c);
int v_insert_nl(struct v_state *v);
int v_backspace(struct v_state *v);

/* src/row.c */
int v_render_row(struct v_row *row);
int v_insert_row(struct v_state *v, int y, char *s, size_t len);
int v_del_row(struct v_state *v, int y);
int v_free_rows(struct v_state *v);
int v_row_insert_char(struct v_row *row, int at, int c);
int v_row_append_str(struct v_row *row, char *s, size_t len);
int v_row_del_char(struct v_row *row, int x);

#endif	/* VOID_H */
