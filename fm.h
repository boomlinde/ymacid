#ifndef _FM_H_
#define _FM_H_

#include "types.h"

struct fm_op {
	u8 level;
	u8 attack;
	u8 decay;
	u8 sustain;
	u8 release;
	u8 ks_attenuation;
	u8 multiplication;
	u8 tremolo;
	u8 vibrato;
	u8 sustain_enabled;
	u8 ksr;
	u8 waveform;
	u8 accent_level;
	u8 accent;
	u8 daccent;
	u8 oh;
	u8 ht;
	u8 att;
};

struct fm_ins {
	struct fm_op ops[2];
	u8 feedback;
	u8 chsettings;
	u8 type;
	float pitch;
	u8 gate;
	u8 pitch_offset;
};

struct fm_voice {
	u8 channel;
	struct fm_ins set;
	struct fm_ins old;
};

struct fm_drumgates {
	u8 bd;
	u8 hh;
	u8 sd;
	u8 tt;
	u8 cy;
};

struct fm_drums {
	struct fm_drumgates set;
	struct fm_drumgates old;
	struct fm_voice bd;
	struct fm_voice hh_sd;
	struct fm_voice tt_cy;
	u8 accent;
};

void fm_init(void);

void fm_initvoice(struct fm_voice *s, u8 channel);
void fm_flush(struct fm_voice *s);

void fm_initdrums(struct fm_drums *s);
void fm_drumflush(struct fm_drums *d);

extern u16 fm_port;
extern u8 fm_opl3;
extern u8 fm_split;

#endif /* _FM_H_ */
