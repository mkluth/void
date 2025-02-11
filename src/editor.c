#include <void.h>

/*
 * v_insert_char - Insert a char into the specified v_state
 * v: pointer to v_state struct
 * c: character to be inserted
 *
 * Description:
 * Returns the newly updated position of v->cur_x (the newly updated cursor's x
 * position) on successful completion. -1 shall be returned if a failure
 * occurred during the insertion process. This function shall also add a new
 * v_row struct inside the v->rows if needed, let's say if there's a new line
 * added by the user. This function is much preferred to be use as it is a lot
 * higher level compared to v_row_insert_char().
 */
int v_insert_char(struct v_state *v, int c)
{
	if (!v)
		return -1;

	if (v->cur_y == v->nrows)
		if (v_append_row(v, "", 0) == -1)
			return -1;

	if (v_row_insert_char(&v->rows[v->cur_y], v->cur_x, c) == -1)
		return -1;

	v->cur_x++;
	v->v_unsaved = V_TRUE;

	return v->cur_x;
}
