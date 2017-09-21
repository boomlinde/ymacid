#include "basspat.h"

#include <stdlib.h>

#include "types.h"
#include "gfx.h"

#define GET(field, step) (((step) >> (field)) & 1)

#define NOTE 0xf
#define BASE_NOTE 40

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

u8 get_note(u8 step)
{
	return (step & NOTE) + BASE_NOTE + 12*GET(OCTPLUS, step) - 12*GET(OCTMIN, step);
}

u8 next_index(struct pattern *pat, u8 index)
{
	return ++index >= pat->length ? 0 : index;
}

static void
toggledraw(struct pattern *pat, u8 field, u8 index)
{
	static u8 ch;
	static u8 en;
	static u8 col;

	en = GET(field, pat->steps[index]);
	if (index >= pat->length) {
		ch = ' ';
		col = COL(RED, BLACK);
	} else if (en) {
		ch = '-';
		col = COL(WHITE, RED);
	} else {
		ch = '|';
		col = (index & 3) ? COL(RED, BLACK) : COL(WHITE, BLACK);
	}

	gfx_plot(2*index + XOFFSET+3, YOFFSET+10 + field, ch, col);
	gfx_plot(2*index + XOFFSET+4, YOFFSET+10 + field, ' ', col);
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
	static u8 ch;
	static u8 step;
	static u8 length;
	static u8 col;

	step = pat->steps[index];
	note = step & NOTE;
	length = pat->length;

	for (i = 0; i < 13; i++) {
		if (index >= length) {
			ch = ' ';
			col = DC;
		} else if (12 - i == note) {
			ch = '-';
			col = COL(WHITE, BLUE);
		} else {
			ch = '|';
			col = (index & 3) ? COL(BLUE, BLACK) : COL(WHITE, BLACK);
		}

		gfx_plot(2*index + XOFFSET+4, YOFFSET+i, ' ', col);
		gfx_plot(2*index + XOFFSET+3, YOFFSET+i, ch, col);
	}

}

void bass_draw(struct pattern *pat)
{
	static u8 i;

	for (i = 0; i < MAXLEN; i++) {
		notedraw(pat, i);
		togglesdraw(pat, i);
	}
}

void bass_toggle(struct pattern *pat, u8 field, u8 index)
{
	pat->steps[index] ^= (1 << field);
	toggledraw(pat, field, index);
}

void bass_setnote(struct pattern *pat, u8 note, u8 index)
{
	static u8 step;

	step = pat->steps[index];
	step = (step & ~NOTE) + note;
	pat->steps[index] = step;
	notedraw(pat, index);
}

void bass_inclength(struct pattern *pat)
{
	if (pat->length < MAXLEN)
		pat->length++;
	notedraw(pat, pat->length - 1);
	togglesdraw(pat, pat->length - 1);
}

void bass_declength(struct pattern *pat)
{
	if (pat->length > 1)
		pat->length--;
	notedraw(pat, pat->length - 1);
	togglesdraw(pat, pat->length - 1);
}

void bass_randomize(struct pattern *pat, u8 index)
{
	u8 step;

	step = (rand() | (1 << ACCENT)) & rand() & rand() & ~NOTE;
	step += rand() & 3 ? rand() % 13 : 0xf;

	pat->steps[index] = step;
}

static void
uidraw(void)
{
	static u8 i;
	static u8 col;
	static const char *const notes[13] = {
		"C ", "C#", "D ", "D#", "E ", "F ", "F#",
		"G ", "G#", "A ", "A#", "B ", "C "
	};

	for (i = 0; i < 13; i++) {
		if (notes[12 - i][1] == '#') {
			col = COL(WHITE, BLACK);
		} else {
			col = COL(BLACK, WHITE);
		}
		gfx_plotstr(XOFFSET+1, YOFFSET+i, notes[12 - i], col);
	}

	gfx_plotstr(XOFFSET+1, YOFFSET+14, "O+\nO-\nSL\nAC", COL(RED, BLACK));
}

void bass_editordraw(struct pattern *pat)
{
	uidraw();
	bass_draw(pat);
}
