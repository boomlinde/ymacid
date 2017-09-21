#include "gfx.h"
#include "types.h"
#include <stdio.h>

static char screen[GFX_WIDTH][GFX_HEIGHT];
static char colors[GFX_WIDTH][GFX_HEIGHT];

void gfx_cls(void)
{
	static u8 x;
	static u8 y;

	for (y = 0; y < GFX_HEIGHT; y++)
		for (x = 0; x < GFX_WIDTH; x++) {
			screen[x][y] = ' ';
			colors[x][y] = DC;
		}
}

void gfx_plot(u8 x, u8 y, char ch, u8 col)
{
	screen[x][y] = ch;
	colors[x][y] = col;
}

void gfx_plotstr(u8 x, u8 y, const char *str, u8 col)
{
	u8 ox;

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

void gfx_refresh(void)
{
	static u8 x;
	static u8 y;
	static u8 col;

	printf("+----------------------------------------+\n");
	for (y = 0; y < 25; y++) {
		printf("|");
		for (x = 0; x < 40; x++) {
			col = colors[x][y];
			printf("\e[1;%d;%dm", (col >> 4) + 30, (col & 0xf) + 40);
			printf("%c", screen[x][y]);
		}
		printf("\e[0m|\n");
	}
	printf("+----------------------------------------+\n");
}
