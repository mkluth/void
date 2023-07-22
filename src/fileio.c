#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <void.h>

/*
 * v_open - Open a file and save its content
 * v: a pointer to a v_state struct
 * filename: the filename to be open
 *
 * Description:
 * Returns V_OK upon successful completion. Otherwise, V_ERR. The file will be
 * opened (if exists) before its content being read and saved inside the
 * specified v_state.
 */
int v_open(struct v_state *v, char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (!fp)
		return V_ERR;

	char *s = NULL;
	size_t cap = 0;
	ssize_t len = getline(&s, &cap, fp);
	if (len == -1) {
		fclose(fp);
		return V_ERR;
	}

	while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r'))
		len--;

	v->row.len = len;
	v->row.cont = malloc(len + 1);
	if (!v->row.cont)
		return V_ERR;

	memcpy(v->row.cont, s, len);
	v->row.cont[len] = '\0';
	v->nrows = 1;

	fclose(fp);
	free(s);
	s = NULL;

	return V_OK;
}
