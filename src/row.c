#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <void.h>

static int v_render_row(struct v_row *row)
{
	int tabs = 0;
	for (int i = 0; i < row->len; i++)
		if (row->orig[i] == '\t')
			tabs++;
	free(row->ren);
	row->ren = malloc(row->len + tabs * (V_TABSTP - 1) + 1);
	if (!row->ren)
		return V_ERR;

	int idx = 0;
	for (int i = 0; i < row->len; i++) {
		if (row->orig[i] != '\t') {
			row->ren[idx++] = row->orig[i];
			continue;
		}
		row->ren[idx++] = ' ';
		while (idx % V_TABSTP)
			row->ren[idx++] = ' ';
	}

	row->ren[idx] = '\0';
	row->rlen = idx;

	return V_OK;
}

/*
 * v_append_row - Append a new v_row into v_state
 * v: pointer to v_state struct
 * s: string to be appended
 * len: length of string s
 *
 * Description:
 * Returns the newly updated number of rows upon successful completion.
 * Otherwise, -1 shall be returned instead. To easily free all of the memories
 * associated with the v->rows array, checkout v_free_rows().
 */
int v_append_row(struct v_state *v, char *s, int len)
{
	if (!v || !s || len < 0)
		return -1;

	struct v_row *tmp = realloc(v->rows,
			sizeof(struct v_row) * (v->nrows + 1));
	if (!tmp)
		return -1;

	v->rows = tmp;
	tmp = NULL;
	struct v_row *row = &v->rows[v->nrows];
	row->len = len;
	row->orig = malloc(len + 1);
	if (!row->orig)
		return -1;

	memcpy(row->orig, s, len);
	row->orig[len] = '\0';
	row->ren = NULL;
	row->rlen = 0;
	if (v_render_row(&v->rows[v->nrows]) == V_ERR)
		return V_ERR;

	v->nrows++;
	row = NULL;

	return v->nrows;
}

/*
 * v_free_rows - Free v_row allocated memory inside the specified v_state
 * v: pointer to v_state struct
 *
 * Description:
 * Returns V_OK upon successful completion. Otherwise, V_ERR shall be returned
 * instead. This function will deallocate all of the related v_row allocated
 * memory inside a v_state, especially on the v->rows array.
 */
int v_free_rows(struct v_state *v)
{
	if (!v || !v->rows || v->nrows < 0)
		return V_ERR;

	while (v->nrows--) {
		struct v_row *row = &v->rows[v->nrows];
		free(row->orig);
		free(row->ren);
		row->orig = NULL;
		row->ren = NULL;
		row->len = 0;
		row->rlen = 0;
		row = NULL;
	}

	free(v->rows);
	v->rows = NULL;
	v->rowoff = 0;
	v->coloff = 0;

	return V_OK;
}

/*
 * v_row_insert_char - Inserts a single char into a v_row at a given position
 * row: pointer to the targeted v_row struct
 * at: index to insert the char into
 * c: the char to be inserted
 *
 * Description:
 * Returns the newly updated length of the v_row's original string. -1 shall
 * be returned instead if failed. The newly updated original string will
 * rendered automatically by v_render_row() before this function exits.
 */
int v_row_insert_char(struct v_row *row, int at, int c)
{
	if (at < 0 || at > row->len)
		at = row->len;

	char *tmp = realloc(row->orig, row->len + 2);
	if (!tmp)
		return -1;

	row->orig = tmp;
	memmove(&row->orig[at + 1], &row->orig[at], row->len - at + 1);
	row->len++;
	row->orig[at] = c;
	v_render_row(row);

	return row->len;
}
