#ifndef _MOUSE_H_
#define _MOUSE_H_

#include "types.h"

extern u8 mouse_enabled;

u8 mouse_init(void);
void mouse_motion(i16 *x, i16 *y);
void mouse_bye(void);

#endif /* _MOUSE_H_ */
