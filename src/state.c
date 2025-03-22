#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include <void.h>

/**
 * v_new_state - create a new v_state struct
 *
 * Create a new v_state struct. The returned struct pointer must be freed, since
 * it is allocated by malloc(). Check out v_dstr_state() for that.
 *
 * Returns a pointer to a v_state struct on success, NULL otherwise.
 */
struct v_state *v_new_state(void)
{
	struct v_state *v = malloc(sizeof(struct v_state));
	if (!v)
		return NULL;

	v->rows = NULL;
	v->nrows = 0;
	v->scr_x = 0;
	v->scr_y = 0;
	v->cur_x = 0;
	v->cur_y = 0;
	v->rcur_x = 0;
	v->rowoff = 0;
	v->coloff = 0;
	v->colors = false;
	v->filename = NULL;
	memset(v->stats_msg, 0, sizeof(v->stats_msg));
	v->dirty = false;
	v->mode = V_CMD;
	v->run = true;

	return v;
}

/**
 * v_dstr_state - destroy the specified v_state struct
 * v: Pointer to the targeted v_state struct.
 *
 * Destroy the specified v_state struct. The specified v_state struct will be
 * deallocates by this function. Do note that this function will automatically
 * calls v_reset_term().
 *
 * Returns V_OK on success, V_ERR otherwise.
 */
int v_dstr_state(struct v_state *v)
{
	if (!v)
		return V_ERR;

	v_reset_term(v);
	v_free_rows(v);
	memset(v->stats_msg, 0, sizeof(v->stats_msg));
	free(v->filename);
	v->filename = NULL;
	v->dirty = false;
	v->mode = V_CMD;
	v->run = false;

	free(v);
	v = NULL;

	return V_OK;
}
