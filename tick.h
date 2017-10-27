#ifndef _TICK_H_
#define _TICK_H_

#include "types.h"

#define TICK_PCSPKR 0
#define TICK_INT15H 1

extern u8 tick_mode;

void tick_init(void);
void tick_settempo(u8 bpm);
void tick_exit(void);
u8 tick(void);

#endif /* _TICK_H_ */
