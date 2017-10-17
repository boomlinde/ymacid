#include <conio.h>
#include <dos.h>

#include "keyboard.h"

#include "types.h"

int cbreak(void) { return 1; }

void keyboard_init(void)
{
	ctrlbrk(cbreak);
	setcbrk(1);
}

/*
 * kbhit/getch combo does not work because they print
 * on ctrl+c/ctrl+brk. The BIOS routines don't print.
 */
int keyboard_poll(void)
{
	_AH = 0x6;
	_DL = 0xff;
	geninterrupt(0x21);
	if (_FLAGS & 0x40) return 0;
	if (_AL == 0) {
		_AH = 0x7;
		geninterrupt(0x21);
		return _AL | 0x100;
	}

	return _AL;
}
