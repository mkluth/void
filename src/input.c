#include <stddef.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

#include <void.h>

static int v_cur_move(struct v_state *v, int key)
{
	struct v_row *row = (v->cur_y >= v->nrows) ? NULL : &v->rows[v->cur_y];

	switch (key) {
	case CUR_LEFT:
		/* Cursor left */
		if (v->cur_x != 0) {
			v->cur_x--;
		} else if (v->cur_y > 0) {
			v->cur_y--;
			v->cur_x = v->rows[v->cur_y].len;
		}
		break;
	case CUR_RIGHT:
		/* Cursor right */
		if (row && v->cur_x < row->len) {
			v->cur_x++;
		} else if (row && v->cur_x == row->len) {
			v->cur_y++;
			v->cur_x = 0;
		}
		break;
	case CUR_UP:
		/* Cursor up */
		if (v->cur_y != 0)
			v->cur_y--;
		break;
	case CUR_DOWN:
		/* Cursor down */
		if (v->cur_y < v->nrows)
			v->cur_y++;
		break;
	case CUR_SOL:
		/* Jump cursor to the start of a line */
		v->cur_x = 0;
		return V_OK;
	case CUR_EOL:
		/* Jump cursor to the end of a line */
		if (row)
			v->cur_x = row->len;
		return V_OK;
	default:
		/* Other keys */
		return V_ERR;
	}

	row = (v->cur_y >= v->nrows) ? NULL : &v->rows[v->cur_y];
	int len = row ? row->len : 0;
	if (v->cur_x > len)
		v->cur_x = len;

	return V_OK;
}

static int v_nav_key(struct v_state *v, int key)
{
	int times = v->scr_y;

	switch (key) {
	case KEY_LEFT:
		/* Arrow Left */
		return v_cur_move(v, CUR_LEFT);
	case KEY_RIGHT:
		/* Arrow Right */
		return v_cur_move(v, CUR_RIGHT);
	case KEY_UP:
		/* Arrow Up */
		return v_cur_move(v, CUR_UP);
	case KEY_DOWN:
		/* Arrow Down */
		return v_cur_move(v, CUR_DOWN);
	case KEY_HOME:
		/* Home Key */
		return v_cur_move(v, CUR_SOL);
	case KEY_END:
		/* End Key */
		return v_cur_move(v, CUR_EOL);
	case KEY_PPAGE:
	case CTRL('y'):
		/* Page Up */
		v->cur_y = v->rowoff;
		goto page_up;
	case KEY_NPAGE:
	case CTRL('v'):
		/* Page Down */
		v->cur_y = v->rowoff + v->scr_x - 1;
		if (v->cur_y > v->nrows)
			v->cur_y = v->nrows;
		goto page_down;
	}

	return V_ERR;

page_up:
	while (times--)
		v_cur_move(v, CUR_UP);
	return V_OK;

page_down:
	while (times--)
		v_cur_move(v, CUR_DOWN);
	return V_OK;
}

static void v_exit(struct v_state *v)
{
	if (v->dirty)
		goto confirm_exit;

	v->v_run = false;
	return;

confirm_exit:
	v_set_stats_msg(v, "WARNING: Unsaved changes. Press again to confirm.");
	v_rfsh_scr(v);
	if (wgetch(v->v_win) != CTRL('q')) {
		v_set_stats_msg(v, "");
		return;
	}

	v->v_run = false;
}

static int v_cmd_input(struct v_state *v, int key)
{
	if (v_cur_move(v, key) == V_OK || v_nav_key(v, key) == V_OK)
		return V_OK;

	switch (key) {
	case CTRL('q'):
		/* Ctrl-Q: Exit the editor */
		v_exit(v);
		return V_OK;
	case 'i':
	case 'I':
		/* i or I: Switch into Insert Mode */
		v->v_mode = V_INSERT;
		v_set_stats_msg(v, "-- INSERT --");
		return V_OK;
	case 'x':
	case 'X':
	case KEY_DC:
		/*
		 * x or X or DELETE:
		 * Delete the character at the cursor's position
		 */
		v_row_del_char(&v->rows[v->cur_y], v->cur_x);
		v->dirty = true;
		return V_OK;
	case CTRL('s'):
		/* Ctrl-S: Write out to a file */
		v_save(v);
		return V_OK;
	case CTRL('l'):
		/* Ctrl-L: Force refresh the editor screen */
		v_rfsh_scr(v);
		return V_OK;
	}

	return V_ERR;
}

static int v_insert_input(struct v_state *v, int key)
{
	if (v_nav_key(v, key) == V_OK)
		return V_OK;

	switch (key) {
	case V_KEY_ESC:
		/* ESC or Ctrl-[: Switch into Command Mode */
		v->v_mode = V_CMD;
		v_set_stats_msg(v, "");
		return V_OK;
	case V_KEY_NL:
	case V_KEY_RET:
		/* ENTER: Insert a newline */
		v_insert_nl(v);
		return V_OK;
	case KEY_BACKSPACE:
	case V_KEY_BKSP:
	case CTRL('h'):
		/* BACKSPACE or Ctrl-H: Delete a character on the leftside */
		v_backspace(v);
		return V_OK;
	case KEY_DC:
		/* DELETE: Delete the character at the cursor's position */
		v_row_del_char(&v->rows[v->cur_y], v->cur_x);
		v->dirty = true;
		return V_OK;
	case CTRL('l'):
		/* Ctrl-L: Force refresh the editor screen */
		v_rfsh_scr(v);
		return V_OK;
	}

	if (v_insert(v, key) != V_ERR)
		return V_OK;

	return V_ERR;
}

/**
 * v_prcs_key - read a key and process it for the specified v_state
 * v: Pointer to the targeted v_state struct.
 *
 * Read a key and process it for the specified v_state. The input processing is
 * done according to the specified v_state current editor mode. Please take note
 * that the v_state curses window, v->v_win is required as we will only read the
 * key on that v_state specific curses window rather than on the stdscr one.
 *
 * Returns V_OK on success, V_ERR otherwise.
 */
int v_prcs_key(struct v_state *v)
{
	if (!v || !v->v_win)
		return V_ERR;

	int stats = V_OK;
	int key = wgetch(v->v_win);

	switch (v->v_mode) {
	case V_CMD:
		/* Command Mode */
		stats = v_cmd_input(v, key);
		break;
	case V_INSERT:
		/* Insert Mode */
		stats = v_insert_input(v, key);
		break;
	default:
		/* Unknown Mode */
		stats = V_ERR;
		break;
	}

	return stats;
}

static int get_prompt_input(WINDOW *win, char *buf, size_t *bufsz,
		size_t *buflen)
{
	int c = wgetch(win);

	switch (c) {
	case KEY_BACKSPACE:
	case KEY_DC:
	case V_KEY_BKSP:
	case CTRL('h'):
		if (*buflen != 0)
			buf[--(*buflen)] = '\0';
		return V_OK;
	case V_KEY_ESC:
		/* Signal the caller to abort the input reading loop */
		return 1;
	case V_KEY_NL:
	case V_KEY_RET:
		/* Signal the caller to return the buf */
		return 2;
	}

	if (iscntrl(c) || c > 128)
		/* Signal the caller that the input given is an invalid key */
		return 3;

	if (*buflen == *bufsz - 1) {
		*bufsz *= 2;
		char *tmp = realloc(buf, *bufsz);
		if (!tmp)
			return V_ERR;

		buf = tmp;
		tmp = NULL;
	}

	buf[*buflen] = c;
	(*buflen)++;
	buf[*buflen] = '\0';

	return V_OK;
}

/**
 * v_prompt - display a prompt in the status bar and get user input
 * v: Pointer to the targeted v_state struct.
 * s: The prompt message.
 *
 * Display a prompt in the status bar and get user input. The default buffer
 * size will be allocated for the first time depends on the value of
 * V_DEFAULT_BUF_SZ macro. However, this function will realloc() the buffer
 * memory when needed. Once the prompt is displayed, this function will reads
 * all of the user keypresses. To submit the input, the user should press
 * the ENTER key to do so. Pressing ESC key or Ctrl-[ will abort the prompt
 * input reading. Keypresses like Ctrl keys (except for Ctrl-H and Ctrl-[) will
 * be ignored, the same thing applied for keys with an integer value above 128.
 * Pressing keys such as Ctrl-H, BACKSPACE key and DELETE key will backspaces
 * the input character.
 *
 * Returns a pointer to a newly allocated string on success, a NULL pointer
 * otherwise. Make sure to free() that pointer once unused.
 */
char *v_prompt(struct v_state *v, char *s)
{
	size_t bufsz = V_DEFAULT_BUF_SZ;

	char *buf = malloc(bufsz);
	if (!buf)
		return NULL;

	size_t buflen = 0;
	buf[0] = '\0';
	int stats = V_OK;

	for (;;) {
		v_set_stats_msg(v, s, buf);
		v_rfsh_scr(v);

		/*
		 * get_prompt_input() will returns different return values
		 * depending on the input given by the user. The values and it's
		 * corresponding meanings are as follows:
		 *
		 * =====	===============================================
		 * VALUE			DESCRIPTION
		 * =====	===============================================
		 *   1		The user pressed the ESC key. Meaning that the
		 *   		user wants to cancel the input reading process.
		 *
		 *   2		The user pressed the ENTER key. Meaning that the
		 *   		user wants to submit the written input.
		 *
		 *   3		The user pressed a key that is either a Ctrl key
		 *   		or a key above the value of 128 (keys that we
		 *   		didn't support for this editor currently).
		 *
		 *  V_OK	The user pressed a key that we can accept.
		 *
		 * V_ERR	An error happened during buffer reallocation
		 *  		process.
		 */
		stats = get_prompt_input(v->v_win, buf, &bufsz, &buflen);
		if (stats == 1 || stats == V_ERR)
			goto stop_prompt;

		if (stats == 2)
			break;
	}

	v_set_stats_msg(v, "");
	return buf;

stop_prompt:
	free(buf);
	buf = NULL;
	v_set_stats_msg(v, "");

	return NULL;
}
