#include "tick.h"

#include <dos.h>

#define PIT_RATE (1193182L>>1)

static u8 laststate;
static u8 count;

void tick_init(void)
{
	outportb(0x61, inportb(0x61) | 1);
}

void tick_settempo(u8 bpm)
{
	static u16 period;

	period = 60 * PIT_RATE / (24 * bpm);
	outportb(0x43, 0xb6);
	outportb(0x42, period);
	outportb(0x42, period >> 8);
}

u8 tick(void)
{
	static u8 state;

	state = inportb(0x61) & (1 << 5);
	if (state && (laststate != state)) {
		laststate = state;
		return (count++ & 1) == 0;
	}

	laststate = state;
	return 0;
}
