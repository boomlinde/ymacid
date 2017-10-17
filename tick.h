#ifndef _TICK_H_
#define _TICK_H_

#include "types.h"

void tick_init(void);
void tick_settempo(u8 bpm);
u8 tick(void);

#endif /* _TICK_H_ */
