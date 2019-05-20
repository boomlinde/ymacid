#include "mouse.h"

#include <dos.h>

u8 mouse_enabled = 0;

static union REGS in;
static union REGS out;

u8 mouse_init(void)
{
	if (!mouse_enabled) return 0;

	in.x.ax = 0;
	int86(0x33, &in, &out);
	return out.x.ax == 0;
}

void mouse_motion(i16 *x, i16 *y)
{
	if (!mouse_enabled) return;

	in.x.ax = 0xb;
	int86(0x33, &in, &out);
	if (x) *x = *(i16 *)&out.x.cx;
	if (y) *y = *(i16 *)&out.x.dx;
}

void mouse_bye(void)
{
	if (!mouse_enabled) return;

	in.x.ax = 0x1f;
	int86(0x33, &in, &out);
}
