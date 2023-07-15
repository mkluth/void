#include <stdlib.h>
#include <ncurses.h>
#include <void.h>

int main(void)
{
	struct v_state *v = v_new_state();
	v_scr_raw(v);
	while (getch() != CTRL('q'));
	v_dstr_state(v);

	return EXIT_SUCCESS;
}
