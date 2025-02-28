#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

#include <void.h>

/*
 * v_open - open a file and load its content into the editor buffer
 * v: Pointer to the targeted v_state struct.
 * filename: The name of the targeted file.
 *
 * Open a file and load its content into the editor buffer. For now, this
 * function only works if the target file already exist. All of the file
 * content will be saves into v->rows array and ready for any kinds of text
 * manipulation.
 *
 * Returns V_OK on success, V_ERR otherwise.
 */
int v_open(struct v_state *v, char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (!fp)
		return V_ERR;

	v->filename = strdup(filename);
	char *s = NULL;
	size_t cap = 0;
	ssize_t len = 0;
	while ((len = getline(&s, &cap, fp)) != -1) {
		while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r'))
			len--;
		if (v_append_row(v, s, len) == V_ERR)
			goto error;
	}

	fclose(fp);
	fp = NULL;
	free(s);
	s = NULL;
	v->dirty = false;

	return V_OK;

error:
	fclose(fp);
	fp = NULL;
	free(s);
	s = NULL;
	v->dirty = false;

	return V_ERR;
}

static char *v_rows_to_str(struct v_state *v, int *buf_len)
{
	int i = 0;
	int total_len = 0;

	for (i = 0; i < v->nrows; i++)
		total_len += v->rows[i].len + 1;
	*buf_len = total_len;

	char *buf = malloc(sizeof(char) * total_len);
	if (!buf)
		return NULL;

	char *p = buf;
	for (i = 0; i < v->nrows; i++) {
		memcpy(p, v->rows[i].orig, v->rows[i].len);
		p += v->rows[i].len;
		*p = '\n';
		p++;
	}

	return buf;
}

/*
 * v_save - save file to disk
 * v: Pointer to the targeted v_state struct.
 *
 * Save file to disk. This function will converts the entire file content into
 * one long ass piece of string. Once converted this function will dumps
 * everything out into that file and thereby saving it. After the saving part is
 * done without any errors occured, the editor dirty flag will flicks
 * automatically to false. Signaling that all changes have been saved. Do note
 * that, this function is intended to be use while the editor curses window
 * still on.
 *
 * Returns V_OK alongside with a status message saying that the changes have
 * been written out successfully, an error message will be displays and V_ERR
 * will be returns otherwise.
 */
int v_save(struct v_state *v)
{
	if (!v || !v->filename)
		return V_ERR;

	int fd = 0;
	char *content = NULL;
	int len = 0;

	content = v_rows_to_str(v, &len);
	if (!content)
		return V_ERR;

	fd = open(v->filename, O_RDWR | O_CREAT, V_FILE_MODE);
	if (fd == -1)
		goto cleanup;

	if (ftruncate(fd, len) == -1)
		goto cleanup;

	if (write(fd, content, len) != len)
		goto cleanup;

	close(fd);
	free(content);
	content = NULL;
	v->dirty = false;

	v_set_stats_msg(v, "%dL %dB written out to disk", v->nrows, len);

	return V_OK;

cleanup:
	if (fd != -1)
		close(fd);
	free(content);
	content = NULL;
	v->dirty = true;

	v_set_stats_msg(v, "ERR: %s", strerror(errno));

	return V_ERR;
}
