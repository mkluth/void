/*
 * row.c - v_row struct manipulation routines
 *
 * This file provides low-level routines for manipulating v_row structs,
 * including rendering, insertion, deletion, deallocation, character insertion,
 * string appending, and character deletion. These routines are more specific
 * and low-level compared to the higher-level operation routines provided in
 * editor.c.
 *
 * Parts of this file are based on the kilo text editor by Salvatore Sanfilippo
 * and Paige Ruten (snaptoken)'s Build Your Own Text Editor booklet:
 *	Copyright (c) 2016 Salvatore Sanfilippo <antirez@gmail.com>
 *	Copyright (c) 2017 Paige Ruten <paige.ruten@gmail.com>
 *
 * Current development and maintenance by:
 * 	Copyright (c) 2025-Present Luth <https://github.com/mkluth>
 *
 * This file is a part of the void text editor.
 * It is licensed under MIT License. See the LICENSE file for details.
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <void.h>

/**
 * v_render_row - render the given v_row struct
 * v: Pointer to the targeted v_state struct.
 * row: Pointer to the targeted v_row struct.
 *
 * Render the given v_row struct. This function renders the content of the
 * specified v_row so that it could be displayed nicely on the editor screen.
 * The rendering for now only focuses on the tab character. Depending on the
 * value of V_TABSTP macro, a tab character will be rendered to match the
 * value of it. The rendered string result will be saved inside row->ren
 * meanwhile the length of the rendered string will be saved inside row->rlen.
 * Dirty flag will be setted to true.
 *
 * Returns V_OK on success, V_ERR otherwise.
 */
int v_render_row(struct v_state *v, struct v_row *row)
{
	int tabs = 0;
	for (int i = 0; i < row->len; i++)
		if (row->orig[i] == '\t')
			tabs++;
	free(row->ren);
	v->dirty = true;
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

/**
 * v_insert_row - insert a new v_row into the specified v_state rows array
 * v: Pointer to the targeted v_state struct.
 * y: The insertion index inside v->rows.
 * s: String to be save.
 * len: Length of string s.
 *
 * Insert a brand new v_row struct into the specified v_state rows array.
 * Do note that by calling this function, v->dirty flag will be setted to
 * true automatically because this function had completely no idea of what
 * you're trying to achieve when it comes to inserting a brand new v_row. It
 * would just assume that you wanted to add a new line for editing purposes,
 * so it flicks on the v->dirty flag. Make sure the given len is big enough to
 * store the string s. Insertion in the middle of v->rows array is supported
 * here.
 *
 * Returns the updated number of v->nrows on success, V_ERR otherwise.
 */
int v_insert_row(struct v_state *v, int y, char *s, size_t len)
{
	if (!v || !s || y < 0 || y > v->nrows)
		return V_ERR;

	struct v_row *tmp = realloc(v->rows,
			sizeof(struct v_row) * (v->nrows + 1));
	if (!tmp)
		return V_ERR;

	v->rows = tmp;
	tmp = NULL;

	memmove(&v->rows[y + 1], &v->rows[y],
		sizeof(struct v_row) * (v->nrows - y));

	struct v_row *row = &v->rows[y];
	row->len = len;
	v->dirty = true;
	row->orig = malloc(len + 1);
	if (!row->orig)
		return V_ERR;

	memcpy(row->orig, s, len);
	row->orig[len] = '\0';
	row->ren = NULL;
	row->rlen = 0;
	if (v_render_row(v, row) == V_ERR)
		return V_ERR;

	v->nrows++;
	row = NULL;

	return v->nrows;
}

/**
 * v_del_row - delete a v_row struct from a v_state rows array
 * v: Pointer to the targeted v_state struct.
 * y: The index of the targeted v_row struct inside v->rows.
 *
 * Deletes the specified v_row struct from the targeted v_state's rows
 * array. This function shall free() the allocated memories of the specified
 * v_row and sets them to NULL. The length of the v_row strings also will be
 * setted to 0. The entire v->rows array shall be move, overlapping the deleted
 * one. The editor dirty flag also will be flicked to true.
 *
 * Returns the newly updated number of v->nrows on success, V_ERR otherwise.
 */
int v_del_row(struct v_state *v, int y)
{
	if (!v || !v->rows || y < 0 || y >= v->nrows)
		return V_ERR;

	struct v_row *row = &v->rows[y];
	free(row->orig);
	free(row->ren);

	row->orig = NULL;
	row->ren = NULL;
	row->len = 0;
	row->rlen = 0;

	memmove(row, &v->rows[y + 1],
		sizeof(struct v_row) * (v->nrows - y - 1));

	v->nrows--;
	v->dirty = true;

	return v->nrows;
}

/**
 * v_free_rows - free the entire rows array inside the specified v_state
 * v: Pointer to the targeted v_state struct.
 *
 * Free the entire rows array inside the specified v_state. v->dirty flag shall
 * be setted to false. Use this function whenever you find the need to free()
 * everything.
 *
 * Returns V_OK on success, V_ERR otherwise.
 */
int v_free_rows(struct v_state *v)
{
	if (!v || !v->rows)
		return V_OK;

	if (v->nrows < 0)
		return V_ERR;

	for (int i = 0; i < v->nrows; i++) {
		struct v_row *row = &v->rows[i];
		free(row->orig);
		free(row->ren);
		row->orig = NULL;
		row->ren = NULL;
		row->len = 0;
		row->rlen = 0;
		row = NULL;
	}

	v->nrows = 0;
	free(v->rows);
	v->rows = NULL;
	v->dirty = false;

	return V_OK;
}

/**
 * v_row_insert_char - insert a char into a v_row at the given position
 * v: Pointer to the targeted v_state struct.
 * row: Pointer to the targeted v_row struct.
 * x: The index to insert the char into.
 * c: Char to be inserted with.
 *
 * Insert a char into a v_row at the given position. This function will
 * realloc() the memory of the specified v_row string automatically before
 * the insertion is carried out. All of the characters within the original
 * string will be moved to create a room for the new character insertion.
 * The original string then will be rendered automatically. Please take note
 * that this function will turn on the editor dirty flag.
 *
 * Returns newly updated number of row->len on success, V_ERR otherwise.
 */
int v_row_insert_char(struct v_state *v, struct v_row *row, int x, int c)
{
	if (x < 0 || x > row->len)
		x = row->len;

	char *tmp = realloc(row->orig, row->len + 2);
	if (!tmp)
		return V_ERR;

	row->orig = tmp;
	memmove(&row->orig[x + 1], &row->orig[x], row->len - x + 1);
	row->len++;
	row->orig[x] = c;
	v->dirty = true;
	v_render_row(v, row);

	return row->len;
}

/**
 * v_row_append_str - append a string to the end of a v_row struct string
 * v: Pointer to the targeted v_state struct.
 * row: Pointer to the targeted v_row struct.
 * s: String to appended with.
 * len: The length of string s.
 *
 * Append a string to the end of a v_row struct string. Used primarily for the
 * purpose of joining two v_row strings together into one. The common use
 * example would be backspacing at the beginning of a line. This is just the
 * common use case of this function, feel free to use it whenever needed. In
 * the end this function simply append the string s to the specified v_row's
 * original string before rendering them and that's all it does. Please take
 * note that this function will turn on the editor dirty flag.
 *
 * Returns the new length of the original updated string on success, V_ERR
 * otherwise.
 */
int v_row_append_str(struct v_state *v, struct v_row *row, char *s, size_t len)
{
	if (!row || !s)
		return V_ERR;

	char *tmp = realloc(row->orig, row->len + len + 1);
	if (!tmp)
		return V_ERR;

	row->orig = tmp;
	memcpy(&row->orig[row->len], s, len);
	row->len += len;
	row->orig[row->len] = '\0';
	v->dirty = true;

	if (v_render_row(v, row) == V_ERR)
		return V_ERR;

	return row->len;
}

/**
 * v_row_del_char - delete a char inside the specified v_row at a given position
 * v: Pointer to the targeted v_state struct.
 * row: Pointer to the targeted v_row struct.
 * x: Index to delete the char from.
 *
 * Delete a char inside the specified v_row at a given position. We don't
 * technically delete the char actually. We simply just move the entire string
 * so that it overlaps that one character we wanted to delete out. Then, we
 * simply decrement the original string length and re-render it. Again, please
 * take note that this function will *turn* on the editor dirty flag.
 *
 * Returns the newly updated value of row->len on success, V_ERR otherwise.
 */
int v_row_del_char(struct v_state *v, struct v_row *row, int x)
{
	if (x < 0 || x >= row->len)
		return V_ERR;

	memmove(&row->orig[x], &row->orig[x + 1], row->len - x);
	row->len--;
	v->dirty = true;
	v_render_row(v, row);

	return row->len;
}
