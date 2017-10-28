#include "tick.h"

#include <dos.h>
#include <stdio.h>

static u8 laststate;
static volatile u8 flag;
static u8 tempo;
static union REGS in;
static union REGS out;
static struct SREGS seg;
u8 tick_mode = TICK_PCSPKR;

#define PIT_RATE (1193182L>>1)

static void settimer(void)
{
	static u32 rate;
	static volatile u8 far *flagptr;

	flagptr = &flag;

	rate = (60 * 1000000L) / (tempo * 48);

	in.x.ax = 0x8301;
	int86(0x15, &in, &out);

	in.x.ax = 0x8300;
	in.x.dx = rate & 0xffff;
	in.x.cx = rate >> 16;
	seg.es = FP_SEG(flagptr);
	in.x.bx = FP_OFF(flagptr);
	int86x(0x15, &in, &out, &seg);
}

void tick_init(void)
{
	switch (tick_mode) {
	case TICK_INT15H:
		in.x.ax = 0x8301;
		int86(0x15, &in, &out);
		tick_settempo(120);
		settimer();
		break;
	case TICK_PCSPKR:
		outportb(0x61, inportb(0x61) | 1);
		break;
	}
}

void tick_settempo(u8 bpm)
{
	static u16 period;

	switch (tick_mode) {
	case TICK_INT15H:
		tempo = bpm;
		break;
	case TICK_PCSPKR:
		period = 60 * PIT_RATE / (24 * bpm);
		outportb(0x43, 0xb6);
		outportb(0x42, period);
		outportb(0x42, period >> 8);
		break;
	}
}

u8 tick(void)
{
	static u8 ret;
	static u8 state;

	switch (tick_mode) {
	case TICK_INT15H:
		ret = flag;
		if (flag) {
			flag = 0;
			tick_settempo(tempo);
			settimer();
		}

		return ret;
	case TICK_PCSPKR:
		state = inportb(0x61) & (1 << 5);
		if (state && (laststate != state)) {
			laststate = state;
			return 1;
		}

		laststate = state;
		return 0;
	}
}

void tick_exit(void)
{
	switch (tick_mode) {
	case TICK_INT15H:
		in.x.ax = 0x8301;
		int86(0x15, &in, &out);
		break;
	case TICK_PCSPKR:
		break;
	}
}
