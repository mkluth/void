#include <stdbool.h>

#include <void.h>

/*
 * v_insert_char - Insert a char into a v_state's v_row
 * v: pointer to v_state struct
 * c: character to be inserted
 *
 * Description:
 * Returns the newly updated position of v->cur_x (the newly updated cursor's x
 * position) on successful completion. V_ERR shall be returned if a failure
 * occurred during the insertion process. This function shall also add a new
 * v_row struct inside the v->rows if needed, let's say if there's a new line
 * added by the user. This function is much preferred to be use as it is a lot
 * higher level compared to v_row_insert_char(). This function will do the
 * insertion process based on the x-value and the y-value of the cursor. Do
 * note that when calling this function, the v->dirty flag will be setted to
 * true automatically.
 */
int v_insert_char(struct v_state *v, int c)
{
	if (!v)
		return V_ERR;

	if (v->cur_y == v->nrows)
		if (v_append_row(v, "", 0) == V_ERR)
			return V_ERR;

	if (v_row_insert_char(v, &v->rows[v->cur_y], v->cur_x, c) == V_ERR)
		return V_ERR;

	v->cur_x++;
	v->dirty = true;

	return v->cur_x;
}

/*
 * v_backspace - backspacing at the targeted v_state
 * v: Pointer to the targeted v_state struct.
 *
 * Backspace a character from a v_state's rows according to the current values
 * of cursor x-position and y-position. There are two types of backspacing can
 * be done here:
 *
 * 	1) Backspacing the character located at the cursor's left.
 * 	2) Backspacing at the beginning of current line.
 *
 * To delete a character on the right or underneath the cursor, you must move
 * the cursor forward beforehand or use v_row_del_char() to directly deletes the
 * character at that specific location. Once this function is called, the
 * v->dirty flag will be setted to true automatically. Unlike most v_row struct
 * manipulation functions, this function will updates automatically the new
 * cursor position depending on which operation is carried out.
 *
 * Depending on what kind of operation is done, this function may returns
 * different context value. For normal backspacing, this function shall returns
 * the newly updated value of v->cur_x whereas backspacing at the beginning of a
 * line shall returns the newly updated value of v->cur_y. Always refer to where
 * the cursor positioning is whenever using this function. V_ERR will be returns
 * otherwise.
 */
int v_backspace(struct v_state *v)
{
	if (!v || v->cur_y == v->nrows || (v->cur_x == 0 && v->cur_y == 0))
		return V_ERR;

	struct v_row *row = &v->rows[v->cur_y];
	if (v->cur_x > 0)
		goto left_backspace;

	v->cur_x = v->rows[v->cur_y - 1].len;
	v_row_append_str(v, v->cur_y - 1, row->orig, row->len);
	v_del_row(v, v->cur_y);
	v->cur_y--;

	return v->cur_y;

left_backspace:
	if (v_row_del_char(v, row, v->cur_x - 1) == V_ERR)
		return V_ERR;
	v->cur_x--;
	v->dirty = true;

	return v->cur_x;
}
