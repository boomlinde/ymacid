#include <dos.h>
#include <time.h>

#include "midi.h"
#include "types.h"
#include "tick.h"

u16 midi_port = 0;

#define STATUS (midi_port + 1)
#define COMMAND STATUS
#define DATA (midi_port)

#define CMD_RESET 0xff
#define CMD_UART 0x3f
#define DATA_ACK 0xfe
#define DSR (1 << 7)
#define DRR (1 << 6)

#define MIDI_START 0xfa
#define MIDI_STOP 0xfc
#define MIDI_CLOCK 0xf8

static u8 mtick;
static u8 playing;

/* Clear incoming data */
static void flush(void)
{
	/* Read and discard input bytes while DATA SET READY is clear */
	while (!(inportb(STATUS) & DSR)) inportb(DATA);

	/* Wait until DRR is clear */
	while (inportb(STATUS) & DRR);
}

void midi_init(void)
{
	time_t starttime;

	if (!midi_port) return;

	/* Reset and wait for ack
	 * Because we can not be sure to get a response on some
	 * cards that don't implement CMD_RESET, time out after a while.
	 */
	starttime = time(0) + 1;
	outportb(COMMAND, CMD_RESET);
	while (inportb(DATA) != DATA_ACK) {
		if (time(0) > starttime) break;
	}

	/* Start UART mode, do not wait for ack! */
	outportb(COMMAND, CMD_UART);
}

void midi_start(void)
{
	if (!midi_port) return;
	flush();

	outportb(DATA, MIDI_START);

	mtick = 0;

	/* If there's a tick, clear it now */
	while (tick());
}

void midi_tick(void)
{
	if (!midi_port) return;
	flush();

	/* Only tick at 24 ppqn derived from the 48 ppqn clock, i.e.
	 * every other step.
	 * */
	if (!mtick) {
		outportb(DATA, MIDI_CLOCK);
	}

	mtick = !mtick;
}

void midi_stop(void)
{
	if (!midi_port) return;
	flush();

	outportb(DATA, MIDI_STOP);
}

void midi_startstop(void) {
	if (playing) {
		midi_stop();
		playing = 0;
	} else {
		midi_start();
		playing = 1;
	}
}
