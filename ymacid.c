#include <stdio.h>  /* for fopen/fwrite/fread */
#include <stdlib.h> /* for rand */
#include <time.h>   /* for time */

#include "gfx.h"
#include "basspat.h"
#include "types.h"
#include "keyboard.h"
#include "tick.h"
#include "fm.h"
#include "bass_seq.h"
#include "drum_seq.h"
#include "edit.h"
#include "pattern.h"
#include "drumpat.h"
#include "theme.h"
#include "cfg.h"

#if 0
#define KBTEST
#endif

#define BASS_MODE 0
#define DRUM_MODE 1

static void set_basspattern(u8 index);
static void savestate(struct state_t *s);
static u8 loadstate(struct state_t *s);
static void bassinput(int key);
static void togglemode(void);
static void setmode(u8 mode);
static void set_drumpattern(u8 index);
static void druminput(int key);
static void drawplaystep(u8 x, u8 y, u8 step, u8 col, u8 playing);

struct state_t {
	u8 mode;
	u8 tempo;
	u8 bpat;
	u8 dpat;
	u8 shuffle;
	u8 doshuffle;
	int tune;
	struct pattern baspat[16];
	struct pattern drumpat[16];
	struct fm_voice bass;
	struct fm_drums drums;
	struct pattern bcopybuf;
	struct pattern dcopybuf;
	struct fm_drumgates mutes;
};

static struct bseq_state bseq;
static struct dseq_state dseq;
static struct state_t s;

int main(void)
{
	static u8 i;
	static u8 j;
	static int key;
	static u8 quit;
	static u8 loaded;
	static u8 ps;

	keyboard_init();

#ifdef KBTEST
	for (;;) {
		key = keyboard_poll();
		if (key) {
			printf("%04x\n", key);
			if (key == 'q') {
				return 0;
			}
		}
	}
#endif

	config_load("ymacid.cfg");
	loaded = loadstate(&s);
	if (!loaded) {
		/* clear state, in case it is partially loaded */
		memset(&s, 0, sizeof (s));
	}

	/* Init */
	gfx_init();
	srand(time(0));
	tick_init();
	fm_init();
	bseq_init(&bseq, &s.bass,
			&s.baspat[s.bpat], &s.tune, &s.shuffle, &s.doshuffle);
	dseq_init(&dseq, &s.drums, &s.drumpat[s.dpat], &s.mutes, &s.shuffle);

	/* default settings */
	if (!loaded) {
		/* Set 16 step patterns for all */
		for (i = 0; i < 16; i++) {
			s.baspat[i].length = 16;
			s.drumpat[i].length = 16;
			for (j = 0; j < 16; j++) {
				s.baspat[i].steps[j] = 0xf;
				s.baspat[i].steps[j] = 0xf;
			}
		}

		s.tempo = 120;
		s.shuffle = 12;
		s.doshuffle = 0;

		fm_initvoice(&s.bass, 0);
		fm_initdrums(&s.drums);

		/* Set up voice */
		s.bass.set.feedback = 2;
		s.bass.set.ops[0].sustain_enabled = 1;
		s.bass.set.ops[0].attack = 10;
		s.bass.set.ops[0].decay = 8;
		s.bass.set.ops[0].sustain = 0;
		s.bass.set.ops[0].release = 6;
		s.bass.set.ops[0].level = 31;
		s.bass.set.ops[0].multiplication = 3;
		s.bass.set.ops[0].waveform = 0;
		s.bass.set.ops[0].accent_level = 0x10;

		s.bass.set.ops[0].sustain_enabled = 0;
		s.bass.set.ops[1].attack = 15;
		s.bass.set.ops[1].decay = 4;
		s.bass.set.ops[1].sustain = 0;
		s.bass.set.ops[1].release = 8;
		s.bass.set.ops[1].level = 0x3b;
		s.bass.set.ops[1].multiplication = 1;
		s.bass.set.ops[0].waveform = 0;

		/* Drums */
		s.drums.bd.set.pitch_offset = 7;
		s.drums.hh_sd.set.pitch_offset = 12;
		s.drums.tt_cy.set.pitch_offset = 12;

		s.drums.bd.set.chsettings = 0xf;
		s.drums.hh_sd.set.chsettings = 0xf;
		s.drums.tt_cy.set.chsettings = 0xf;

		s.drums.bd.set.ops[0].attack = 0xf;
		s.drums.bd.set.ops[1].attack = 0xf;
		s.drums.hh_sd.set.ops[0].attack = 0xf;
		s.drums.hh_sd.set.ops[1].attack = 0xf;
		s.drums.tt_cy.set.ops[0].attack = 0xf;
		s.drums.tt_cy.set.ops[1].attack = 0xf;

		s.drums.bd.set.ops[0].decay = 0x9;
		s.drums.bd.set.ops[1].decay = 0x6;
		s.drums.hh_sd.set.ops[0].decay = 0x9;
		s.drums.hh_sd.set.ops[1].decay = 0x8;
		s.drums.tt_cy.set.ops[0].decay = 0x8;
		s.drums.tt_cy.set.ops[1].decay = 0x4;

		s.drums.bd.set.ops[0].release = 0xf;
		s.drums.bd.set.ops[1].release = 0xf;
		s.drums.hh_sd.set.ops[0].release = 0xf;
		s.drums.hh_sd.set.ops[1].release = 0xf;
		s.drums.tt_cy.set.ops[0].release = 0xf;
		s.drums.tt_cy.set.ops[1].release = 0xf;

		s.drums.bd.set.ops[0].level = 0x3b;
		s.drums.bd.set.ops[1].level = 0x3f;
		s.drums.hh_sd.set.ops[0].level = 0x2f;
		s.drums.hh_sd.set.ops[1].level = 0x38;
		s.drums.tt_cy.set.ops[0].level = 0x38;
		s.drums.tt_cy.set.ops[1].level = 0x20;

		s.drums.bd.set.ops[0].multiplication = 0;
		s.drums.bd.set.ops[1].multiplication = 1;

		s.drums.bd.set.pitch = 28.0;
		s.drums.hh_sd.set.pitch = 43.0;
		s.drums.tt_cy.set.pitch = 45.0;
	}

	/* Limit waveforms if opl3 is not enabled */
	if (!fm_opl3) {
		s.bass.set.ops[0].waveform &= 3;
		s.bass.set.ops[1].waveform &= 3;
	}

	if (fm_split) {
		s.bass.set.chsettings = 0xa;
		s.drums.bd.set.chsettings = 0x5;
		s.drums.hh_sd.set.chsettings = 0x5;
		s.drums.tt_cy.set.chsettings = 0x5;
	} else {
		s.bass.set.chsettings = 0xf;
		s.drums.bd.set.chsettings = 0xf;
		s.drums.hh_sd.set.chsettings = 0xf;
		s.drums.tt_cy.set.chsettings = 0xf;
	}

	s.drums.accent = 0;
	s.drums.bd.set.ops[0].att = 0;
	s.drums.bd.set.ops[1].att = 0;
	s.drums.hh_sd.set.ops[0].att = 0;
	s.drums.hh_sd.set.ops[1].att = 0;
	s.drums.tt_cy.set.ops[0].att = 0;
	s.drums.tt_cy.set.ops[1].att = 0;
	fm_drumflush(&s.drums);
	fm_flush(&s.bass);
	tick_settempo(s.tempo);

	gfx_cls(theme_cur->bass_dc);
	bass_editordraw(&s.baspat[s.bpat]);
	bass_drawpatname(s.bpat);
	gfx_plotstrf(14, 1, "tempo:%d  ", theme_cur->bass_dc, s.tempo);
	gfx_plotstrf(27, 1, "tune:%d  ", theme_cur->bass_dc, s.tune);
	edit_drawsynth(&s.bass, 0);
	gfx_setpage(1);
	gfx_cls(theme_cur->drum_dc);
	drump_editordraw(&s.drumpat[s.dpat]);
	drump_drawpatname(s.dpat);
	drump_setmute(&s.mutes);
	gfx_plotstrf(14, 1, "tempo:%d  ", theme_cur->drum_dc, s.tempo);
	gfx_plotstrf(25, 1, "shuffle:%d  ", theme_cur->drum_dc, s.shuffle - 12);
	edit_drawdrums(&s.drums, 0);
	setmode(s.mode);

	while (!quit) {
		if (tick()) {
			switch (s.mode) {
			case BASS_MODE:
				if (s.doshuffle) {
					ps = bseq.tick < s.shuffle && bseq.tick > (s.shuffle>>1);
					ps |= bseq.tick >= s.shuffle + (s.shuffle>>1);
				} else {
					ps = bseq.tick % 12 > 4;
				}
				drawplaystep(5, 2,
						(bseq.step<<1)+ps, theme_cur->bass_dc, bseq.playing);
				break;
			case DRUM_MODE:
				ps = dseq.tick < s.shuffle && dseq.tick > (s.shuffle>>1);
				ps |= dseq.tick >= s.shuffle + (s.shuffle>>1);
				drawplaystep(5, 5,
						(dseq.step<<1)+ps, theme_cur->drum_dc, dseq.playing);
				break;
			}
			bseq_tick(&bseq);
			dseq_tick(&dseq);
		}

		key = keyboard_poll();
		if (!key) continue;

		switch (key) {
		case 0x11: quit = 1; break; /* ctrl+q */
		case '\t': togglemode(); break;
		case '\r': bseq_startstop(&bseq); dseq_startstop(&dseq); break;
		case 0x149: if (s.tempo < 200) tick_settempo(++s.tempo); break;
		case 0x151: if (s.tempo >  30) tick_settempo(--s.tempo); break;
		default:
			switch (s.mode) {
			case BASS_MODE: bassinput(key); break;
			case DRUM_MODE: druminput(key); break;
			}
		}

		/* Pass input directly for parameters */
		switch (s.mode) {
		case BASS_MODE:
			edit_drawsynth(&s.bass, key);
			edit_drawsynth(&s.bass, 0);
			gfx_plotstrf(14, 1, "tempo:%d  ", theme_cur->bass_dc, s.tempo);
			gfx_plotstrf(27, 1, "tune:%d  ", theme_cur->bass_dc, s.tune);
			break;
		case DRUM_MODE:
			edit_drawdrums(&s.drums, key);
			edit_drawdrums(&s.drums, 0);
			gfx_plotstrf(14, 1, "tempo:%d  ", theme_cur->drum_dc, s.tempo);
			gfx_plotstrf(25, 1, "shuffle:%d  ",
					theme_cur->drum_dc, s.shuffle-12);
			if (s.doshuffle) gfx_plotstr(25, 2, "(both)", theme_cur->drum_dc);
			else gfx_plotstr(25, 2, "      ", theme_cur->drum_dc);
			break;
		}
	}

	/* Turn gates off and save state */
	s.bass.set.gate = 0;
	s.bass.set.ops[0].accent = 0;
	fm_flush(&s.bass);
	memset(&s.bass.old, 0xff, sizeof (s.bass.old));
	s.bass.old.pitch = 128.0;

	memset(&s.drums.set, 0, sizeof (s.drums.set));
	fm_drumflush(&s.drums);
	memset(&s.drums.old, 0xff, sizeof (s.drums.old));
	memset(&s.drums.bd.old, 0xff, sizeof (s.drums.bd.old));
	memset(&s.drums.hh_sd.old, 0xff, sizeof (s.drums.hh_sd.old));
	memset(&s.drums.tt_cy.old, 0xff, sizeof (s.drums.tt_cy.old));

	s.drums.bd.old.pitch = 128.0;
	s.drums.hh_sd.old.pitch = 128.0;
	s.drums.tt_cy.old.pitch = 128.0;

	savestate(&s);
	gfx_reset();

	tick_exit();

	return 0;
}

static void
set_drumpattern(u8 index)
{
	drump_clearcursor(&s.drumpat[s.dpat]);
	s.dpat = index;
	dseq.next = &s.drumpat[s.dpat];
	drump_draw(&s.drumpat[s.dpat]);
	drump_drawpatname(index);
}

static void
set_basspattern(u8 index)
{
	bass_clearcursor(&s.baspat[s.bpat]);
	s.bpat = index;
	bseq.next = &s.baspat[s.bpat];
	bass_draw(&s.baspat[s.bpat]);
	bass_drawpatname(index);
}

static void
savestate(struct state_t *s)
{
	FILE *fp;

	fp = fopen("ymacid.sav", "wb");
	if (!fp) return;

	fwrite(s, sizeof (*s), 1, fp);
	fclose(fp);
}

static u8
loadstate(struct state_t *s)
{
	static size_t n;
	FILE *fp;

	fp = fopen("ymacid.sav", "rb");
	if (fp == 0) return 0;

	/*
	 * Can't read 1 sizeof (*s) element for some reason,
	 * but sizeof (*s) single byte elements works fine,
	 * so no complaints there.
	 */
	n = fread(s, 1, sizeof (*s), fp);
	if (n != sizeof (*s)) return 0;
	return fclose(fp) == 0;
}

static void
bassinput(int key)
{
#define PAT &s.baspat[s.bpat]
	switch (key) {
	case '=': bass_inclength(PAT); break;
	case '-': bass_declength(PAT); break;
	case 0x14b:
	case 'h': bass_deccursor(PAT); break;
	case 0x14d:
	case 'l': bass_inccursor(PAT); break;
	case '4': bass_randomize(PAT); break;
	case 'f': bass_toggle(PAT, OCTPLUS); break;
	case 'd': bass_toggle(PAT, OCTMIN); break;
	case 's': bass_toggle(PAT, SLIDE); break;
	case 'a': bass_toggle(PAT, ACCENT); break;
	case ';': bass_clearstep(PAT); break;
	case 0x173:
	case 'H': bass_rotleft(PAT); break;
	case 0x174:
	case 'L': bass_rotright(PAT); break;
	case '0': bass_setcursor(PAT, 0); break;
	case 0x150:
	case 'j': bass_decnote(PAT); break;
	case 0x148:
	case 'k': bass_incnote(PAT); break;
	case 0x191:
	case 'J': bass_decnotes(PAT); break;
	case 0x18d:
	case 'K': bass_incnotes(PAT); break;
	case 'o': bass_repeat(PAT); break;

	case 'F': bass_clearfield(PAT, OCTPLUS); break;
	case 'D': bass_clearfield(PAT, OCTMIN); break;
	case 'S': bass_clearfield(PAT, SLIDE); break;
	case 'A': bass_clearfield(PAT, ACCENT); break;

	case 'q': bass_setnote(PAT,  0); break;
	case '2': bass_setnote(PAT,  1); break;
	case 'w': bass_setnote(PAT,  2); break;
	case '3': bass_setnote(PAT,  3); break;
	case 'e': bass_setnote(PAT,  4); break;
	case 'r': bass_setnote(PAT,  5); break;
	case '5': bass_setnote(PAT,  6); break;
	case 't': bass_setnote(PAT,  7); break;
	case '6': bass_setnote(PAT,  8); break;
	case 'y': bass_setnote(PAT,  9); break;
	case '7': bass_setnote(PAT, 10); break;
	case 'u': bass_setnote(PAT, 11); break;
	case '1': bass_setnote(PAT, 15); break;

	case 0x13b: set_basspattern( 0); break;
	case 0x13c: set_basspattern( 1); break;
	case 0x13d: set_basspattern( 2); break;
	case 0x13e: set_basspattern( 3); break;
	case 0x13f: set_basspattern( 4); break;
	case 0x140: set_basspattern( 5); break;
	case 0x141: set_basspattern( 6); break;
	case 0x142: set_basspattern( 7); break;
	case 0x154: set_basspattern( 8); break;
	case 0x155: set_basspattern( 9); break;
	case 0x156: set_basspattern(10); break;
	case 0x157: set_basspattern(11); break;
	case 0x158: set_basspattern(12); break;
	case 0x159: set_basspattern(13); break;
	case 0x15a: set_basspattern(14); break;
	case 0x15b: set_basspattern(15); break;

	case 0x3:
		memcpy(&s.bcopybuf, PAT, sizeof (s.bcopybuf));
		break;
	case 0x16:
		if (s.bcopybuf.length == 0) break;
		bass_clearcursor(PAT);
		memcpy(PAT, &s.bcopybuf, sizeof (s.bcopybuf));
		bass_editordraw(PAT);
		break;

	case '+': if (s.tune < 24) s.tune++; break;
	case '_': if (s.tune > -24) s.tune--; break;
	}
#undef PAT
}

static void
druminput(int key)
{
#define PAT &s.drumpat[s.dpat]
#define DECCUR  drump_deccursor(PAT)
#define INCCUR  drump_inccursor(PAT)
	switch (key) {
	case '=': drump_inclength(PAT); break;
	case '-': drump_declength(PAT); break;
	case 0x14b:
	case 'h': drump_deccursor(PAT); break;
	case 0x14d:
	case 'l': drump_inccursor(PAT); break;
	case 0x150:
	case 'j': do INCCUR; while (s.drumpat[s.dpat].cursor & 3); break;
	case 0x148:
	case 'k': do DECCUR; while (s.drumpat[s.dpat].cursor & 3); break;
	case ';': drump_clearstep(PAT); break;
	case 0x173:
	case 'H': drump_rotleft(PAT); break;
	case 0x174:
	case 'L': drump_rotright(PAT); break;
	case '0': drump_setcursor(PAT, 0); break;
	case 'o': drump_repeat(PAT); break;

	case '1': drump_toggle(PAT, BDBIT); break;
	case '2': drump_toggle(PAT, SDBIT); break;
	case '3': drump_toggle(PAT, CHBIT); break;
	case '4': drump_toggle(PAT, OHBIT); break;
	case '5': drump_toggle(PAT, LTBIT); break;
	case '6': drump_toggle(PAT, HTBIT); break;
	case '7': drump_toggle(PAT, CYBIT); break;
	case '8': drump_toggle(PAT, ACBIT); break;

	case '!': drump_clearfield(PAT, BDBIT); break;
	case '@': drump_clearfield(PAT, SDBIT); break;
	case '#': drump_clearfield(PAT, CHBIT); break;
	case '$': drump_clearfield(PAT, OHBIT); break;
	case '%': drump_clearfield(PAT, LTBIT); break;
	case '^': drump_clearfield(PAT, HTBIT); break;
	case '&': drump_clearfield(PAT, CYBIT); break;
	case '*': drump_clearfield(PAT, ACBIT); break;

	case 'Q':
		s.mutes.bd = ~s.mutes.bd;
		drump_setmute(&s.mutes); break;
	case 'W':
		s.mutes.sd = ~s.mutes.sd;
		drump_setmute(&s.mutes); break;
	case 'E':
		s.mutes.hh = ~s.mutes.hh;
		drump_setmute(&s.mutes); break;
	case 'T':
		s.mutes.tt = ~s.mutes.tt;
		drump_setmute(&s.mutes); break;
	case 'U':
		s.mutes.cy = ~s.mutes.cy;
		drump_setmute(&s.mutes); break;

	case 0x13b: set_drumpattern( 0); break;
	case 0x13c: set_drumpattern( 1); break;
	case 0x13d: set_drumpattern( 2); break;
	case 0x13e: set_drumpattern( 3); break;
	case 0x13f: set_drumpattern( 4); break;
	case 0x140: set_drumpattern( 5); break;
	case 0x141: set_drumpattern( 6); break;
	case 0x142: set_drumpattern( 7); break;
	case 0x154: set_drumpattern( 8); break;
	case 0x155: set_drumpattern( 9); break;
	case 0x156: set_drumpattern(10); break;
	case 0x157: set_drumpattern(11); break;
	case 0x158: set_drumpattern(12); break;
	case 0x159: set_drumpattern(13); break;
	case 0x15a: set_drumpattern(14); break;
	case 0x15b: set_drumpattern(15); break;

	case 0x3:
		memcpy(&s.dcopybuf, PAT, sizeof (s.dcopybuf));
		break;
	case 0x16:
		if (s.dcopybuf.length == 0) break;
		drump_clearcursor(PAT);
		memcpy(PAT, &s.dcopybuf, sizeof (s.dcopybuf));
		drump_editordraw(PAT);
		break;

	case '+': if (s.shuffle < 16) s.shuffle++; break;
	case '_': if (s.shuffle > 12) s.shuffle--; break;
	case '?': s.doshuffle = !s.doshuffle; break;
	}
#undef PAT
#undef DECCUR
#undef INCCUR
}

static void togglemode(void)
{
	if (s.mode == BASS_MODE) setmode(DRUM_MODE);
	else setmode(BASS_MODE);
}
static void setmode(u8 mode)
{
	s.mode = mode;
	gfx_setpage(mode);
}

static void drawplaystep(u8 x, u8 y, u8 step, u8 col, u8 playing)
{
	gfx_plotstr(x, y, "                                ", col);
	if (playing) gfx_plotstr(x+step, y, "V", col);
}
