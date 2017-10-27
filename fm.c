#include "fm.h"

#include <string.h>
#include <stdio.h>
#include <math.h>

#include "types.h"

#define CSW_NOTESEL           0x08
#define TREM_VIB_SUS_KSD_FACT 0x20
#define KSLEV_OUTLEV          0x40
#define AD                    0x60
#define SR                    0x80
#define FNUMLO                0xA0
#define KEY_BLOCK_FNUMHI      0xB0
#define TREM_VIB_DEPTH_PERC   0xBD
#define FB_TYPE               0xC0
#define WAVEFORM              0xE0

#define CLOCK 49716.0
#define BLOCK(x) ((1L<<(20-(x))) / CLOCK)
#define OUT(blk) return (((u16)(0.5+freq*BLOCK(blk))) | ((blk) << 10))

u16 fm_port = 0x220;
u8 fm_opl3 = 0;
u8 fm_split = 0;

static u16
f2fnum(float freq)
{
    if (BLOCK(0)*freq < 1023.0) OUT(0);
    if (BLOCK(1)*freq < 1023.0) OUT(1);
    if (BLOCK(2)*freq < 1023.0) OUT(2);
    if (BLOCK(2)*freq < 1023.0) OUT(2);
    if (BLOCK(3)*freq < 1023.0) OUT(3);
    if (BLOCK(4)*freq < 1023.0) OUT(4);
    if (BLOCK(5)*freq < 1023.0) OUT(5);
    if (BLOCK(6)*freq < 1023.0) OUT(6);
    if (BLOCK(7)*freq < 1023.0) OUT(7);
	return 0x1fff;
}

static u16
midi2fnum(float note)
{
	/*
	 * Normally, this would be 440 * pow(2, (note - 69)/12), but due to a quirk
	 * in the FP system that makes it only produce correct pow results for
	 * integer exponents, the base had to be changed according to the rules
	 * below:
	 *
	 * pow(2, x/12) == pow(pow(2, 1/12), x)
	 * pow(2, 1/12) == 1.0594630943592953
	 * pow(2, x/12) == pow(1.0594630943592953, x)
	 *
	 * This still isn't enough work if we want 1 cent precision. We have to
	 * raise that to a hundredth and pre-multply the note number by 100:
	 *
	 * pow(2, x/12/100) == pow(pow(2, 1/12/100), x)
	 * pow(2, 1/12/100) == 1.0005777895065548
	 * pow(2, x/12/100) == pow(1.0005777895065548, x)
	 * pow(2, x/12) == pow(1.0005777895065548, x*100)
	 * 
	 * Also, 440 has been replaced and the -69 root note offset removed
	 *
	 * let b = 1.0005777895065548 (the 1-cent base)
	 * z * pow(b, m - n) == z * pow(b, -n) * pow(b, m)
	 * 440 * pow(b, m - 6900) == 440 * pow(b, -6900) * pow(b, m)
	 * 440 * pow(b, -6900) == 8.17579891564
	 * 440 * pow(b, m - 6900) == 8.17579891564 * pow(b, m)
	 */
	return f2fnum(8.17579891564*pow(1.0005777895065548, note*100.0));
}

static void fmwrite(u16 reg, u8 data)
{
	static u8 i;
	static u8 add;

	add = reg & 0xff00 ? 2 : 0;
	reg &= 0xff;

	/*
	 * On OPL2 you need to wait 3.3 us after writing an address.
	 * Supposedly, IN on the FM port six times will consistently
	 * take that long because a number of its cycles will have to
	 * pass before being able to respond.
	 */
	outportb(fm_port + add, reg);
	for (i = 0; i < 6; i++) {
		inportb(fm_port + add);
	}

	/* Wait 23 us after a register write */
	outportb(fm_port+1 + add, data);
	for (i = 0; i < 36; i++) {
		inportb(fm_port + add);
	}
}

static void fm_set(u8 reg, u8 ch, u8 op, u8 data)
{
	static u8 opi[9] = {0, 1, 2, 8, 9, 10, 16, 17, 18};
	static u8 opi_opl3[18] = {0, 1, 2, 6, 7, 8, 12, 13, 14, 18, 19, 20, 24, 25, 26, 30, 31, 32};
	static u16 op_offset[36] = {
		0x000,0x001,0x002,0x003,0x004,0x005,
		0x008,0x009,0x00A,0x00B,0x00C,0x00D,
		0x010,0x011,0x012,0x013,0x014,0x015,
		0x100,0x101,0x102,0x103,0x104,0x105,
		0x108,0x109,0x10A,0x10B,0x10C,0x10D,
		0x110,0x111,0x112,0x113,0x114,0x115
	};

	switch (reg) {
	case CSW_NOTESEL:
	case TREM_VIB_DEPTH_PERC:
		fmwrite(reg, data);
		break;
	case FNUMLO:
	case KEY_BLOCK_FNUMHI:
	case FB_TYPE:
		fmwrite(reg + ch, data);
		break;
	default:
		if (fm_opl3) {
			fmwrite(reg + op_offset[opi_opl3[ch] + (op ? 3 : 0)], data);
		} else {
			fmwrite(reg + opi[ch] + (op ? 3 : 0), data);
		}
		break;
	}
}

void fm_initvoice(struct fm_voice *s, u8 channel)
{
	s->channel = channel;
	memset(&s->old, 0xff, sizeof (s->old));

	/* floats can't be all 1s */
	s->old.pitch = 128.0;

	memset(&s->set, 0, sizeof (s->set));
	fm_flush(s);
}

void fm_initdrums(struct fm_drums *d)
{
	memset(&d->old, 0xff, sizeof (d->old));
	memset(&d->set, 0, sizeof (d->old));

	fm_initvoice(&d->bd, 6);
	fm_initvoice(&d->hh_sd, 7);
	fm_initvoice(&d->tt_cy, 8);
	fm_drumflush(d);
}

void fm_init(void)
{
	static u8 i;

	/* Zero out the whole chip */
	for (i = 1; i < 0xf6; i++) {
		fmwrite(i, 0);
		fmwrite(0x100 + i, 0);
	}

	/* Enable non-sine waveforms */
	fmwrite(0x01, 1 << 5);

	if (fm_opl3) {
		fmwrite(0x105, 1);
	}
} 

static void op_flush(u8 op, struct fm_voice *s)
{
#define OLD (s->old.ops[op])
#define SET (s->set.ops[op])
#define FMSET(r, x) fm_set((r), s->channel, op, (x))
#define COMB(hi, lo) (((lo) & 0xf) | (((hi) & 0xf) << 4))
#define CHANGED(x) (OLD.x != SET.x)
#define BIT(n, v) (((v) & 1) << n)
	static u8 level;
	if ((CHANGED(attack) || CHANGED(decay) || CHANGED(oh)) && !SET.accent) {
		if (!SET.oh) FMSET(AD, COMB(SET.attack, SET.decay));
		else FMSET(AD, COMB(SET.attack, 0x7));
	}
	if ((CHANGED(sustain) || CHANGED(release)) && !SET.accent) {
		FMSET(SR, COMB(SET.sustain ^ 0xf, SET.release));
	}
	if ((CHANGED(level)
				|| CHANGED(ks_attenuation)
				|| CHANGED(att))
			&& !SET.accent) {
		level = SET.level;
		if (SET.att) {
			level -= (level >> 3);
		}
		FMSET(KSLEV_OUTLEV, ((~level)&0x3f) | (SET.ks_attenuation<<6));
	}
	if (CHANGED(multiplication)
			|| CHANGED(ksr)
			|| CHANGED(sustain_enabled)
			|| CHANGED(vibrato)
			|| CHANGED(ht)
			|| CHANGED(tremolo)) {
		FMSET(TREM_VIB_SUS_KSD_FACT,
				((SET.ht ? SET.multiplication+1 : SET.multiplication) & 0xf)
				| BIT(4, SET.ksr)
				| BIT(5, SET.sustain_enabled)
				| BIT(6, SET.vibrato)
				| BIT(7, SET.tremolo));
	}
	if (CHANGED(waveform)) {
		FMSET(WAVEFORM, SET.waveform & (fm_opl3 ? 0x7 : 0x3));
	}

	/* On accents, change ASDR to 88f8 and add the accent level */
	if (CHANGED(accent)) {
		if (SET.accent) {
			level = SET.level + (SET.accent_level >> (SET.level >> 4));
			if (level > 0x3f) level = 0x3f;
			FMSET(AD, COMB(8, 8));
			FMSET(SR, COMB(0xf, SET.release));
			FMSET(KSLEV_OUTLEV, (~level)&0x3f | (SET.ks_attenuation<<6));
		} else {
			FMSET(AD, COMB(SET.attack, SET.decay));
			FMSET(SR, COMB(SET.sustain ^ 0xf, SET.release));
			FMSET(KSLEV_OUTLEV, (~(SET.level))&0x3f | (SET.ks_attenuation<<6));
		}
	}
#undef OLD
#undef SET
#undef FMSET
#undef COMB
#undef CHANGED
#undef BIT
}

void fm_flush(struct fm_voice *s)
{
#define FMSET(r, x) fm_set((r), s->channel, 0, (x))
#define CHANGED(x) (s->old.x != s->set.x)
	static u16 fnum;
	op_flush(0, s);
	op_flush(1, s);

	if (CHANGED(feedback) || CHANGED(type) || CHANGED(chsettings)) {
		FMSET(FB_TYPE, ((s->set.feedback & 7) << 1) | (s->set.type & 1) | (s->set.chsettings << 4));
	}
	if (CHANGED(pitch) || CHANGED(gate) || CHANGED(pitch_offset)) {
		fnum = midi2fnum(s->set.pitch + s->set.pitch_offset);
		FMSET(FNUMLO, fnum);
		FMSET(KEY_BLOCK_FNUMHI, ((s->set.gate == 1) << 5) |
				((fnum >> 8) & 0x1f));
	}
	memcpy(&s->old, &s->set, sizeof (s->old));
#undef FMSET
#undef CHANGED
}

void fm_drumflush(struct fm_drums *d)
{
#define SD(bit, _v) ((d->set._v != 0) << bit)
#define OD(bit, _v) ((d->old._v != 0) << bit)
	static u8 oldd;
	static u8 setd;

	setd = SD(0, hh) | SD(1, cy) | SD(2, tt) | SD(3, sd) | SD(4, bd);
	oldd = OD(0, hh) | OD(1, cy) | OD(2, tt) | OD(3, sd) | OD(4, bd);

	if (oldd != setd) {
		if (d->set.bd && !d->old.bd) d->bd.set.ops[1].att    = !d->accent;
		if (d->set.hh && !d->old.hh) d->hh_sd.set.ops[0].att = !d->accent;
		if (d->set.sd && !d->old.sd) d->hh_sd.set.ops[1].att = !d->accent;
		if (d->set.tt && !d->old.tt) d->tt_cy.set.ops[0].att = !d->accent;
		if (d->set.cy && !d->old.cy) d->tt_cy.set.ops[1].att = !d->accent;
	}

	fm_flush(&d->bd);
	fm_flush(&d->hh_sd);
	fm_flush(&d->tt_cy);

	if (oldd != setd) fmwrite(TREM_VIB_DEPTH_PERC, setd | (1 << 5));

	memcpy(&d->old, &d->set, sizeof (d->old));
#undef SD
#undef OD
}
