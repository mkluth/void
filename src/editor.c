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
 * v_del_char - Delete a char from a v_state's v_row
 * v: pointer to v_state struct
 *
 * Description:
 * Returns the newly updated position of v->cur_x (the newly updated cursor's x
 * position) on successful completion. V_ERR shall be returned if a failure
 * occurred during the deletion process. Do note that deletion in this function
 * will only take place on the leftside character located next to the cursor. To
 * delete a character on the rightside, you must move the cursor forward
 * beforehand. This function will do the deletion process based on the x-value
 * and the y-value of the cursor. Do note that when calling this function, the
 * v->dirty flag will be setted to true automatically.
 */
int v_del_char(struct v_state *v)
{
	if (!v || v->cur_y == v->nrows)
		return V_ERR;

	struct v_row *row = &v->rows[v->cur_y];
	if (v->cur_x > 0) {
		if (v_row_del_char(v, row, v->cur_x - 1) == V_ERR)
			return V_ERR;
		v->cur_x--;
		v->dirty = true;
	}

	return v->cur_x;
}
