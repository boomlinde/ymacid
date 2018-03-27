#include <dos.h>
#include <stdarg.h>
#include <stdio.h>

#include "gfx.h"
#include "types.h"

#define VIDEO_BASE 0xb800

static int pageoffset;
static char sprintfbuf[81];

static u8 oldmode;
static u8 oldpage;

void
gfx_init(void)
{
	/* Store old video mode and page */
	_AH = 0xf;
	geninterrupt(0x10);
	oldmode = _AL;
	oldpage = _BH;

	/* Set 40*25 16 color text mode */
	_AH = 0x0;
	_AL = 0x1;
	geninterrupt(0x10);

	/* Disable cursor */
	_AH = 0x1;
	_CX = 0x1000;
	geninterrupt(0x10);

	/* Enable intensive colors */
	_AX = 0x1003;
	_BX = 0;
	geninterrupt(0x10);

	gfx_setpage(0);
}

void
gfx_setpage(u8 page)
{
	_AH = 0x5;
	_AL = page;
	geninterrupt(0x10);
	pageoffset = page << 11;
}

void
gfx_plot(int x, int y, u8 ch, u8 col)
{
	static int offset;
	
	offset = (y << 6) + (y << 4) + (x << 1) + pageoffset;
	pokeb(VIDEO_BASE, offset, ch);
	pokeb(VIDEO_BASE, offset + 1, col);
}

void gfx_reset(void)
{
	gfx_setpage(oldpage);

	/* Set old mode */
	_AH = 0x0;
	_AL = oldmode;
	geninterrupt(0x10);

	/* Normal cursor */
	_AH = 0x1;
	_CX = 0x0607;
	geninterrupt(0x10);
}

void gfx_cls(u8 col)
{
	static u8 x;
	static u8 y;

	for (y = 0; y < GFX_HEIGHT; y++)
		for (x = 0; x < GFX_WIDTH; x++) {
			gfx_plot(x, y, ' ', col);
		}
}

void gfx_plotstr(u8 x, u8 y, const char *str, u8 col)
{
	static u8 ox;

	ox = x;
	while (*str != 0) {
		if (*str == '\n') {
			str++;
			y++;
			x = ox;
		} else {
			gfx_plot(x++, y, *str++, col);
		}
	}
}

void gfx_plotstrf(u8 x, u8 y, const char *fmt, u8 col, ...)
{
	va_list argptr;
	va_start(argptr, fmt);
	vsprintf(sprintfbuf, fmt, argptr);
	va_end(argptr);
	gfx_plotstr(x, y, sprintfbuf, col);
}
