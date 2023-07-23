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
	ssize_t len = 0;
	while ((len = getline(&s, &cap, fp)) != -1) {
		while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r'))
			len--;
		if (v_append_row(v, s, len) == -1)
			goto error;
	}

	fclose(fp);
	fp = NULL;
	free(s);
	s = NULL;

	return V_OK;
error:
	fclose(fp);
	fp = NULL;
	free(s);
	s = NULL;

	return V_ERR;
}
