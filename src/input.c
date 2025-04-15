#include <stddef.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

#include <void.h>

static int v_nav(struct v_state *v, int key)
{
	switch (key) {
	case V_CUR_LEFT:
	case KEY_LEFT:
		/* Cursor left */
		return v_cur_left(v);
	case V_CUR_RGHT:
	case KEY_RIGHT:
		/* Cursor right */
		return v_cur_right(v);
	case V_CUR_UP:
	case KEY_UP:
		/* Cursor up */
		return v_cur_up(v);
	case V_CUR_DOWN:
	case KEY_DOWN:
		/* Cursor down */
		return v_cur_down(v);
	case V_CUR_SOL:
	case KEY_HOME:
		/* Jump to the start of the line */
		return v_cur_bol(v);
	case V_CUR_EOL:
	case KEY_END:
		/* Jump to the end of the line */
		return v_cur_eol(v);
	case KEY_PPAGE:
		/* Page up */
		return v_ppage(v);
	case KEY_NPAGE:
		/* Page down */
		return v_npage(v);
	}

	return V_ERR;
}

static int v_quit(struct v_state *v)
{
	if (!v->dirty)
		goto quit;

	v_set_stats_msg(v, "WARNING: Unsaved changes. Press again to quit.");
	v_rfsh_scr(v);
	int key = getch();
	if (key != V_CTRL_QUIT)
		return V_ERR;

quit:
	v->run = false;
	return V_OK;
}

static const struct v_key ctrls[] = {
	{V_CTRL_QUIT, v_quit},		/* Quit the editor */
	{0, NULL}			/* Sentinel */
};

static int v_ctrl(struct v_state *v, int key)
{
	for (int i = 0; ctrls[i].func; i++)
		if (ctrls[i].key == key)
			return ctrls[i].func(v);

	return V_ERR;
}

/**
 * v_prcs_key - read a key and process it for the specified v_state
 * v: Pointer to the targeted v_state struct.
 *
 * Read a key and process it for the specified v_state. The input processing is
 * done according to the specified v_state current editor mode. Please take note
 * that the curses window must be initialiazed before this function call.
 *
 * Returns V_OK on success, V_ERR otherwise.
 */
int v_prcs_key(struct v_state *v)
{
	if (!v)
		return V_ERR;

	int key = getch();
	if (v_nav(v, key) == V_OK)
		return V_OK;

	if (v_ctrl(v, key) == V_OK)
		return V_OK;

	return V_ERR;
}

static int get_prompt_input(char *buf, size_t *bufsz, size_t *buflen)
{
	int c = getch();

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
 * otherwise. Make sure to free() that returned pointer once unused.
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
		stats = get_prompt_input(buf, &bufsz, &buflen);
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
