/*
 * main.c - The editor's main entry point
 *
 * This file contains the main driving routine for the editor.
 *
 * This editor is supported only on Linux-based systems (e.g., Ubuntu, Arch,
 * Fedora, etc) and macOS. It heavily relies on ncurses functionality. There's
 * no current plan to support PDCurses or native Windows terminal. This editor
 * has also been tested on Termux (Android) and yes, it works fine.
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

#include <stdlib.h>

#include <void.h>

int main(int argc, char **argv)
{
	struct v_state *v = v_new_state();
	v_init_term(v);
	v_init_colors(v);

	if (argc >= 2)
		v_open(v, argv[1]);

	while (v->run) {
		v_rfsh_scr(v);
		v_prcs_key(v);
	}

	v_dstr_state(v);

	return EXIT_SUCCESS;
}
