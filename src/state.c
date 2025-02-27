#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include <void.h>

/*
 * v_new_state - Create a new v_state struct
 *
 * Description:
 * Returns a pointer to a v_state struct, NULL if failed. The returned pointer
 * must be freed, since it is allocated by malloc().
 */
struct v_state *v_new_state(void)
{
	struct v_state *v = malloc(sizeof(struct v_state));
	if (!v)
		return NULL;

	v->v_win = NULL;
	v->rows = NULL;
	v->nrows = 0;
	v->scr_x = 0;
	v->scr_y = 0;
	v->cur_x = 0;
	v->cur_y = 0;
	v->rcur_x = 0;
	v->rowoff = 0;
	v->coloff = 0;
	v->v_colors = false;
	v->filename = NULL;
	memset(v->stats_msg, 0, sizeof(v->stats_msg));
	v->dirty = false;
	v->v_mode = V_CMD;
	v->v_run = true;

	return v;
}

/*
 * v_dstr_state - Destroy the specified v_state struct
 * v: pointer to v_state struct
 *
 * Description:
 * Returns V_OK on success, V_ERR otherwise. The specified v_state struct shall
 * be deallocated.
 */
int v_dstr_state(struct v_state *v)
{
	if (!v)
		return V_ERR;

	if (v->v_win)
		v_reset_term(v);

	v_free_rows(v);
	memset(v->stats_msg, 0, sizeof(v->stats_msg));
	free(v->filename);
	v->filename = NULL;
	v->dirty = false;
	v->v_mode = V_CMD;
	v->v_run = false;

	free(v);
	v = NULL;

	return V_OK;
}
