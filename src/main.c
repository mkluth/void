#include <stdlib.h>
#include <ncurses.h>
#include <void.h>

int main(void)
{
	struct v_state *v = v_new_state();
	v_scr_raw(v);
	while (v->v_run)
		v_prcs_key(v);

	v_dstr_state(v);

	return EXIT_SUCCESS;
}
