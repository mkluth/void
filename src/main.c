#include <stdlib.h>

#include <void.h>

int main(int argc, char **argv)
{
	struct v_state *v = v_new_state();
	v_init_term(v);
	v_init_colors(v);

	if (argc >= 2)
		v_open(v, argv[1]);

	while (v->v_run) {
		v_rfsh_scr(v);
		v_prcs_key(v);
	}

	v_reset_term(v);
	v_dstr_state(v);

	return EXIT_SUCCESS;
}
