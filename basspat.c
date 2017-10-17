#include "basspat.h"

#include <stdlib.h>

#include "types.h"
#include "pattern.h"
#include "gfx.h"
#include "theme.h"

#define GET(field, step) (((step) >> (field)) & 1)

#define XOFFSET 2
#define YOFFSET 2

/*
 * 76543210
 * ||||||||
 * |||||||+- note
 * ||||||+-- note
 * |||||+--- note
 * ||||+---- note
 * |||+----- oct+
 * ||+------ oct-
 * |+------- slide
 * +-------- accent
 */

float basspat_getnote(u8 step)
{
	return (float)(step & NOTE) + 12*GET(OCTPLUS, step) - 12*GET(OCTMIN, step);
}

static void
drawkey(u8 i, u8 curkey)
{
	static u8 col;
	static const char *const notes[13] = {
		"C ", "C#", "D ", "D#", "E ", "F ", "F#",
		"G ", "G#", "A ", "A#", "B "
	};

	i &= NOTE;
	if (i >= 12) return;
	curkey &= NOTE;

	if (notes[i][1] == '#') {
		col = theme_cur->kb_black;
	} else {
		col = theme_cur->kb_white;
	}
	if (i == curkey) {
		col |= 0x88;
	}
	gfx_plotstr(XOFFSET+1, YOFFSET+12-i, notes[i], col);
}

static void
drawkeys(struct pattern *pat)
{
	static u8 i;
	static u8 curkey;

	curkey = pat->steps[pat->cursor];
	for (i = 0; i < 12; i++) {
		drawkey(i, curkey);
	}
}


static void
toggledraw(struct pattern *pat, u8 field, u8 index)
{
	static u8 *str;
	static u8 en;
	static u8 col;

	en = GET(field, pat->steps[index]);
	if (index >= pat->length) {
		str = "  ";
		col = theme_cur->bass_dc;
	} else if (en) {
		str = "- ";
		col = theme_cur->bass_set;
	} else {
		str = "| ";
		col = (index&3) ? theme_cur->bass_step : theme_cur->bass_hlstep;
	}

	gfx_plotstr(2*index + XOFFSET+3, YOFFSET+10 + field, str, col);
}

static void
togglesdraw(struct pattern *pat, u8 index)
{
	toggledraw(pat, OCTPLUS, index);
	toggledraw(pat, OCTMIN, index);
	toggledraw(pat, SLIDE, index);
	toggledraw(pat, ACCENT, index);
}

static void
notedraw(struct pattern *pat, u8 index)
{
	static u8 i;
	static u8 note;
	static u8 *str;
	static u8 step;
	static u8 length;
	static u8 col;

	step = pat->steps[index];
	note = step & NOTE;
	length = pat->length;

	for (i = 0; i < 12; i++) {
		if (index >= length) {
			str = "  ";
			col = theme_cur->bass_dc;
		} else if (11 - i == note) {
			str = "- ";
			col = theme_cur->bass_set;
		} else {
			str = "| ";
			col = (index&3)
				? theme_cur->bass_step
				: theme_cur->bass_hlstep;
		}

		gfx_plotstr(2*index + XOFFSET+3, YOFFSET+1+i, str, col);
	}
}

void bass_draw(struct pattern *pat)
{
	static u8 i;

	for (i = 0; i < MAXLEN; i++) {
		notedraw(pat, i);
		togglesdraw(pat, i);
	}

	drawkeys(pat);
	bass_setcursor(pat, pat->cursor);
}

void bass_toggle(struct pattern *pat, u8 field)
{
	pat->steps[pat->cursor] ^= (1 << field);
	toggledraw(pat, field, pat->cursor);
	bass_inccursor(pat);
}

void bass_setnote(struct pattern *pat, u8 note)
{
	static u8 step;

	step = pat->steps[pat->cursor];
	step = (step & ~NOTE) + note;
	pat->steps[pat->cursor] = step;
	notedraw(pat, pat->cursor);
	bass_inccursor(pat);
}

void bass_inclength(struct pattern *pat)
{
	if (pat->length < MAXLEN) {
		pat->length++;
		notedraw(pat, pat->length - 1);
		togglesdraw(pat, pat->length - 1);
	}
}

void bass_declength(struct pattern *pat)
{
	if (pat->length > 1) {
		pat->length--;
		notedraw(pat, pat->length);
		togglesdraw(pat, pat->length);
		if (pat->cursor >= pat->length) {
			bass_deccursor(pat);
		}
	}
}

void bass_randomize(struct pattern *pat)
{
	u8 step;

	step = (rand() | (1 << ACCENT)) & rand() & rand() & ~NOTE;
	step += rand() & 3 ? rand() % 12 : 0xf;

	pat->steps[pat->cursor] = step;
	notedraw(pat, pat->cursor);
	togglesdraw(pat, pat->cursor);
	bass_inccursor(pat);
}

void bass_inccursor(struct pattern *pat)
{
	if (pat->cursor + 1 < pat->length) {
		bass_setcursor(pat, pat->cursor + 1);
	} else {
		bass_setcursor(pat, 0);
	}
}

void bass_deccursor(struct pattern *pat)
{
	if (pat->cursor > 0) {
		bass_setcursor(pat, pat->cursor - 1);
	} else {
		bass_setcursor(pat, pat->length - 1);
	}
}

void bass_clearstep(struct pattern *pat)
{
	pat->steps[pat->cursor] = 0xf;
	notedraw(pat, pat->cursor);
	togglesdraw(pat, pat->cursor);
	bass_inccursor(pat);
}

static void
uidraw(void)
{
	gfx_plotstr(XOFFSET+1, YOFFSET+14, "O+\nO-\nSL\nAC", theme_cur->bass_dc);
	gfx_plotstr(XOFFSET+3, YOFFSET-1, "pat:", theme_cur->bass_dc);
}

void bass_rotleft(struct pattern *pat)
{
	static u8 buf;
	static u8 length;
	static u8 i;

	length = pat->length;

	if (length == 1) return;

	buf = pat->steps[0];

	for (i = 0; i < length - 1; i++) {
		pat->steps[i] = pat->steps[i+1];
	}

	pat->steps[length - 1] = buf;
	bass_draw(pat);
}

void bass_rotright(struct pattern *pat)
{
	static u8 buf;
	static u8 length;
	static u8 i;

	length = pat->length;

	if (length == 1) return;

	buf = pat->steps[length - 1];

	for (i = length - 1; i > 0; i--) {
		pat->steps[i] = pat->steps[i-1];
	}

	pat->steps[0] = buf;
	bass_draw(pat);
}

void bass_clearcursor(struct pattern *pat)
{
	gfx_plotstr(XOFFSET+3 + (pat->cursor << 1),
			YOFFSET + 13, "  ", theme_cur->bass_dc);
}

void bass_setcursor(struct pattern *pat, u8 index)
{
	gfx_plotstr(XOFFSET+3 + (pat->cursor << 1),
			YOFFSET + 13, "  ", theme_cur->bass_dc);
	pat->cursor = index;
	if (index >= pat->length) index = pat->length - 1;
	gfx_plotstr(XOFFSET+3 + (pat->cursor << 1),
			YOFFSET + 13, "/\\", theme_cur->bass_dc);
	drawkeys(pat);
}

void bass_drawpatname(u8 index)
{
	static const char *s1 = "ffffffffFFFFFFFF";
	static const char *s2 = "1234567812345678";
	gfx_plot(XOFFSET+7, YOFFSET-1, s1[index], theme_cur->bass_dc);
	gfx_plot(XOFFSET+8, YOFFSET-1, s2[index], theme_cur->bass_dc);
}

void bass_incnote(struct pattern *pat)
{
	static u8 note;
	static u8 rest;

	rest = pat->steps[pat->cursor];
	note = rest & 0xf;
	rest = rest & 0xf0;
	if (note == 0xf) return;
	if (++note > 11) note = 0;
	pat->steps[pat->cursor] = rest | note;
	notedraw(pat, pat->cursor);
	drawkeys(pat);
}

void bass_decnote(struct pattern *pat)
{
	static u8 note;
	static u8 rest;

	rest = pat->steps[pat->cursor];
	note = rest & 0xf;
	rest = rest & 0xf0;
	if (note == 0xf) return;
	if (--note > 11) note = 11;
	pat->steps[pat->cursor] = rest | note;
	notedraw(pat, pat->cursor);
	drawkeys(pat);
}

void bass_incnotes(struct pattern *pat)
{
	static u8 i;
	static u8 pos;

	pos = pat->cursor;
	for (i = pos; i < pat->length; i++) {
		bass_incnote(pat);
		bass_inccursor(pat);
	}
	bass_setcursor(pat, pos);
}

void bass_decnotes(struct pattern *pat)
{
	static u8 i;
	static u8 pos;

	pos = pat->cursor;
	for (i = pos; i < pat->length; i++) {
		bass_decnote(pat);
		bass_inccursor(pat);
	}
	bass_setcursor(pat, pos);
}

void bass_repeat(struct pattern *pat)
{
	static u8 n;
	static u8 i;

	n = pat->cursor;

	for (i = 0; i < n; i++) {
		if (n + i >= pat->length) break;

		pat->steps[n + i] = pat->steps[i];
		notedraw(pat, n + i);
		togglesdraw(pat, n + i);
		bass_inccursor(pat);
	}
}

void bass_clearfield(struct pattern *pat, u8 field)
{
	pat->steps[pat->cursor] &= ~(1 << field);
	toggledraw(pat, field, pat->cursor);
	bass_inccursor(pat);
}

void bass_editordraw(struct pattern *pat)
{
	uidraw();
	bass_draw(pat);
}
