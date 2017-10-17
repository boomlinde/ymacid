#ifndef _LIB_H_
#define _LIB_H_

#include "types.h"

#define GFX_WIDTH 40
#define GFX_HEIGHT 25

#define BLACK   0
#define BLUE    1
#define GREEN   2
#define CYAN    3
#define RED     4
#define MAGENTA 5
#define YELLOW  6
#define WHITE   7

#define BRIGHT(c) ((c)|8)
#define COL(fg, bg) (((bg) << 4) + (fg))
#define DC COL(BRIGHT(YELLOW), MAGENTA)
#define CD COL(MAGENTA, BRIGHT(YELLOW))

void gfx_init(void);
void gfx_plot(int x, int y, u8 ch, u8 col);
void gfx_setpage(u8 page);
void gfx_reset(void);
void gfx_cls(u8 col);
void gfx_plotstr(u8 x, u8 y, const char *str, u8 col);
void gfx_plotstrf(u8 x, u8 y, const char *fmt, u8 col, ...);

#endif /* _LIB_H_ */
