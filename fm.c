#include <time.h>
#include <stdlib.h>

#include "gfx.h"
#include "types.h"
#include "basspat.h"

int main(void) {
	static struct pattern pat;
	static u8 i;

	srand(time(NULL));

	pat.length = 16;

	for (i = 0; i < 16; i++) {
		bass_randomize(&pat, i);
	}

	gfx_cls();
	bass_editordraw(&pat);
	gfx_refresh();
}
