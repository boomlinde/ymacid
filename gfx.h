#ifndef _GFX_H_
#define _GFX_H_

#include "types.h"

#define GFX_WIDTH 40
#define GFX_HEIGHT 25

#define BLACK   0
#define RED     1
#define GREEN   2
#define YELLOW  3
#define BLUE    4
#define MAGENTA 5
#define CYAN    6
#define WHITE   7

#define COL(fg, bg) (((fg) << 4) + (bg))

#define DC COL(YELLOW, BLACK)

void gfx_cls(void);
void gfx_plot(u8 x, u8 y, char ch, u8 col);
void gfx_plotstr(u8 x, u8 y, const char *str, u8 col);
void gfx_refresh(void);

#endif /* _GFX_H_ */
