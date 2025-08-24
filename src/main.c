/*
 * main.c - The editor's main entry point
 *
 * This file contains the main driving routine for the editor.
 *
 * Parts of this program are based on the kilo text editor by Salvatore
 * Sanfilippo, Paige Ruten (snaptoken)'s Build Your Own Text Editor booklet and
 * Linus Torvalds's uEmacs/PK 4.0 editor input approach:
 *	Copyright (c) 2016 Salvatore Sanfilippo <antirez@gmail.com>
 *	Copyright (c) 2017 Paige Ruten <paige.ruten@gmail.com>
 *	Copyright (c) 2018 Linus Torvalds <torvalds@linux-foundation.org>
 *
 * Current development and maintenance by:
 * 	Copyright (c) 2025-Present Luth <https://github.com/mkluth>
 *
 * This file is a part of the void text editor.
 * It is licensed under MIT License. See the LICENSE file for details.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <void.h>

static void usage(void)
{
	printf("void %s - %s\n\n", V_VER, V_DESC);
	fputs("Usage: void [arguments] [file]\tEdit specified file.\n\n",
	     stdout);
	fputs("Arguments:\n", stdout);
	fputs("   -h\tDisplay this help and exit.\n", stdout);
	fputs("   -v\tOutput version information and exit.\n", stdout);
	fputs("   -n\tTurns off colors support.\n", stdout);

	exit(EXIT_FAILURE);
}

static void version(void)
{
	printf("void - version %s\n", V_VER);
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	int opt;
	struct v_state *v = v_new_state();
	while ((opt = getopt(argc, argv, "hvn")) != -1) {
		switch (opt) {
		case 'h':
			usage();
			break;
		case 'v':
			version();
			break;
		case 'n':
			/* Open without colors support */
			v->colors = false;
			break;
		default:
			/* Display help and exit */
			v_dstr_state(v);
			usage();
		}
	}

	v_init_term(v);
	if (v->colors)
		v_init_colors(v);

	if (optind < argc)
		v_open(v, argv[optind]);

	while (v->run) {
		v_rfsh_scr(v);
		v_prcs_key(v);
	}

	v_dstr_state(v);

	return EXIT_SUCCESS;
}
