#ifndef _MIDI_H_
#define _MIDI_H_

#include "types.h"

extern u16 midi_port;

void midi_init(void);
void midi_start(void);
void midi_tick(void);
void midi_stop(void);
void midi_startstop(void);

#endif /* _MIDI_H_ */
