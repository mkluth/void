#include <void.h>

static void snap_cur_eol(struct v_state *v)
{
	struct v_row *row = (v->cur_y >= v->nrows) ? NULL : &v->rows[v->cur_y];
	int len = row ? row->len : 0;
	if (v->cur_x > len)
		v->cur_x = len;
}

/**
 * v_cur_left - move cursor to the left
 * v: Pointer to the targeted v_state struct.
 *
 * Move cursor to the left. This function not really moves the cursor, but
 * rather it decrements the value of the cursor x-position. The actual screen
 * update can only be seen once v_rfsh_scr() is called. This function also
 * allows the cursor to move left at the start of a line, making it moves one
 * line up.
 *
 * Returns V_OK no matter what.
 */
int v_cur_left(struct v_state *v)
{
	if (v->cur_x != 0) {
		v->cur_x--;
	} else if (v->cur_y > 0) {
		v->cur_y--;
		v->cur_x = v->rows[v->cur_y].len;
	}

	snap_cur_eol(v);

	return V_OK;
}

/**
 * v_cur_right - move cursor to the right
 * v: Pointer to the targeted v_state struct.
 *
 * Move cursor to the right. This function not really moves the cursor, but
 * rather it increments the value of the cursor x-position. The actual screen
 * update can only be seen once v_rfsh_scr() is called. This function also
 * allows the cursor to move right at the end of a line, making it moves one
 * line down.
 *
 * Returns V_OK no matter what.
 */
int v_cur_right(struct v_state *v)
{
	struct v_row *row = (v->cur_y >= v->nrows) ? NULL : &v->rows[v->cur_y];

	if (row && v->cur_x < row->len) {
		v->cur_x++;
	} else if (row && v->cur_x == row->len) {
		v->cur_y++;
		v->cur_x = 0;
	}

	snap_cur_eol(v);

	return V_OK;
}

/**
 * v_cur_up - move the cursor up
 * v: Pointer to the targeted v_state struct.
 *
 * Move the cursor up. This function not really moves the cursor, but
 * rather it decrements the value of the cursor y-position. The actual screen
 * update can only be seen once v_rfsh_scr() is called. This function will
 * only works if there's a line before the current one.
 *
 * Returns V_OK no matter what.
 */
int v_cur_up(struct v_state *v)
{
	if (v->cur_y != 0)
		v->cur_y--;
	snap_cur_eol(v);

	return V_OK;
}

/**
 * v_cur_down - move the cursor down
 * v: Pointer to the targeted v_state struct.
 *
 * Move the cursor down. This function not really moves the cursor, but
 * rather it increments the value of the cursor y-position. The actual screen
 * update on can only be seen once v_rfsh_scr() is called. This function will
 * only works if there's a line after the current one.
 *
 * Returns V_OK no matter what.
 */
int v_cur_down(struct v_state *v)
{
	if (v->cur_y < v->nrows)
		v->cur_y++;
	snap_cur_eol(v);

	return V_OK;
}

/**
 * v_cur_bol - move the cursor to the beginning of the current line
 * v: Pointer to the targeted v_state struct.
 *
 * Move the cursor to the beginning of the current line. It basically sets the
 * cursor x-position to 0. The actual screen update can only be seen once
 * v_rfsh_scr() is called.
 *
 * Returns V_OK no matter what.
 */
int v_cur_bol(struct v_state *v)
{
	v->cur_x = 0;
	return V_OK;
}

/**
 * v_cur_eol - move the cursor to the end of the current line
 * v: Pointer to the targeted v_state struct.
 *
 * Move the cursor to the end of the current line. It basically sets the
 * cursor x-position to the maximum length of the current line, if exist. The
 * actual screen update can only be seen once v_rfsh_scr() is called.
 *
 * Returns V_OK no matter what.
 */
int v_cur_eol(struct v_state *v)
{
	struct v_row *row = (v->cur_y >= v->nrows) ? NULL : &v->rows[v->cur_y];
	if (v->cur_y < v->nrows)
		v->cur_x = row->len;

	return V_OK;
}

/**
 * v_ppage - page up the cursor
 * v: Pointer to the targeted v_state struct.
 *
 * Page up the cursor. It basically moves the cursor up using v_cur_up()
 * repeatedly. The actual screen update can only be seen once v_rfsh_scr() is
 * called.
 *
 * Returns V_OK no matter what.
 */
int v_ppage(struct v_state *v)
{
	v->cur_y = v->rowoff;
	int times = v->scr_y;

	while (times--)
		v_cur_up(v);

	return V_OK;
}

/**
 * v_npage - page down the cursor
 * v: Pointer to the targeted v_state struct.
 *
 * Page up the cursor. It basically moves the cursor down using v_cur_down()
 * repeatedly. The actual screen update can only be seen once v_rfsh_scr() is
 * called.
 *
 * Returns V_OK no matter what.
 */
int v_npage(struct v_state *v)
{
	v->cur_y = v->rowoff + v->scr_y - 1;
	if (v->cur_y > v->nrows)
		v->cur_y = v->nrows;

	int times = v->scr_y;
	while (times--)
		v_cur_down(v);

	return V_OK;
}

/**
 * v_bottom_pg - go to the bottom of the page
 * v: Pointer to the targeted v_state struct.
 *
 * Go to the bottom of the page. Nothing trivial, it simply changes the cursor
 * position values to the last line of currently opened buffer. The actual
 * screen update can only be seen once v_rfsh_scr() is called.
 *
 * Returns V_OK always.
 */
int v_bottom_pg(struct v_state *v)
{
	v->cur_y = v->nrows - 1;
	v->cur_x = 0;
	return V_OK;
}

/**
 * v_top_pg - go to the top of the page
 * v: Pointer to the targeted v_state struct.
 *
 * Go to the top of the page. Nothing trivial here too, it simply changes the
 * cursor position values to 0. The actual screen update can only be seen once
 * v_rfsh_scr() is called.
 *
 * Returns V_OK always, no matter what.
 */
int v_top_pg(struct v_state *v)
{
	v->cur_y = 0;
	v->cur_x = 0;
	return V_OK;
}
