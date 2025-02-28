#include <stdbool.h>

#include <void.h>

/**
 * v_insert - perform character insertion at the targeted v_state
 * v: Pointer to the targeted v_state struct.
 * c: The character to be inserted with.
 *
 * Perform character insertion at the targeted v_state. Please take note that
 * the insertion here is done according to the current values of the specified
 * v_state cursor's x-position and y-position. For now, there are two types of
 * insertion can be done here:
 *
 * 	1) Insertion when the cursor is on the tilde line after the file's EOL.
 * 	2) Normal character insertion.
 *
 * If the cursor happens to be on the tilde line (past the file's EOL), a new
 * v_row struct will be appended into the v->rows array immediately allowing the
 * insertion to be carried out. The editor dirty flag will be setted to true
 * after the operation. The main difference between v_insert() and
 * v_row_insert_char() would be that v_insert() is more oriented for the
 * editor's operation meanwhile v_row_insert_char() is more focused on a v_row
 * struct strings manipulation. The v_state's cursor position also will be
 * updated before this function exits.
 *
 * Returns the newly updated value of v->cur_x on success, V_ERR otherwise.
 */
int v_insert(struct v_state *v, int c)
{
	if (!v)
		return V_ERR;

	if (v->cur_y == v->nrows)
		if (v_append_row(v, "", 0) == V_ERR)
			return V_ERR;

	if (v_row_insert_char(&v->rows[v->cur_y], v->cur_x, c) == V_ERR)
		return V_ERR;

	v->cur_x++;
	v->dirty = true;

	return v->cur_x;
}

/**
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
	v_row_append_str(&v->rows[v->cur_y - 1], row->orig, row->len);
	v_del_row(v, v->cur_y);
	v->cur_y--;
	v->dirty = true;

	return v->cur_y;

left_backspace:
	if (v_row_del_char(row, v->cur_x - 1) == V_ERR)
		return V_ERR;
	v->cur_x--;
	v->dirty = true;

	return v->cur_x;
}
