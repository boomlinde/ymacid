#include "drumpat.h"

#include <stdlib.h>

#include "types.h"
#include "pattern.h"
#include "gfx.h"
#include "theme.h"

#define GET(field, step) (((step) >> (field)) & 1)

#define XOFFSET 2
#define YOFFSET 6

static void
toggledraw(struct pattern *pat, u8 field, u8 index)
{
	static u8 *str;
	static u8 en;
	static u8 col;

	en = GET(field, pat->steps[index]);
	if (index >= pat->length) {
		str = "  ";
		col = theme_cur->drum_dc;
	} else if (en) {
		str = "- ";
		col = theme_cur->drum_set;
	} else {
		str = "| ";
		col = (index&3) ? theme_cur->drum_step : theme_cur->drum_hlstep;
	}

	gfx_plotstr(2*index + XOFFSET+3, YOFFSET+field, str, col);
}

static void
togglesdraw(struct pattern *pat, u8 index)
{
	toggledraw(pat, BDBIT, index);
	toggledraw(pat, CHBIT, index);
	toggledraw(pat, OHBIT, index);
	toggledraw(pat, SDBIT, index);
	toggledraw(pat, LTBIT, index);
	toggledraw(pat, HTBIT, index);
	toggledraw(pat, CYBIT, index);
	toggledraw(pat, ACBIT, index);
}


void drump_draw(struct pattern *pat)
{
	static u8 i;

	for (i = 0; i < MAXLEN; i++) {
		togglesdraw(pat, i);
	}

	drump_setcursor(pat, pat->cursor);
}

void drump_toggle(struct pattern *pat, u8 field)
{
	pat->steps[pat->cursor] ^= (1 << field);
	toggledraw(pat, field, pat->cursor);
	drump_inccursor(pat);
}

void drump_inclength(struct pattern *pat)
{
	if (pat->length < MAXLEN) {
		pat->length++;
		togglesdraw(pat, pat->length - 1);
	}
}

void drump_declength(struct pattern *pat)
{
	if (pat->length > 1) {
		pat->length--;
		togglesdraw(pat, pat->length);
		if (pat->cursor >= pat->length) {
			drump_deccursor(pat);
		}
	}
}

void drump_inccursor(struct pattern *pat)
{
	if (pat->cursor + 1 < pat->length) {
		drump_setcursor(pat, pat->cursor + 1);
	} else {
		drump_setcursor(pat, 0);
	}
}

void drump_deccursor(struct pattern *pat)
{
	if (pat->cursor > 0) {
		drump_setcursor(pat, pat->cursor - 1);
	} else {
		drump_setcursor(pat, pat->length - 1);
	}
}

void drump_clearstep(struct pattern *pat)
{
	pat->steps[pat->cursor] = 0;
	togglesdraw(pat, pat->cursor);
	drump_inccursor(pat);
}

void drump_rotleft(struct pattern *pat)
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
	drump_draw(pat);
}

void drump_rotright(struct pattern *pat)
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
	drump_draw(pat);
}

void drump_clearcursor(struct pattern *pat)
{
	gfx_plotstr(XOFFSET+3 + (pat->cursor << 1),
			YOFFSET + 8, "  ",
			theme_cur->drum_dc);
}

void drump_setcursor(struct pattern *pat, u8 index)
{
	gfx_plotstr(XOFFSET+3 + (pat->cursor << 1),
			YOFFSET + 8, "  ",
			theme_cur->drum_dc);
	pat->cursor = index;
	if (index >= pat->length) index = pat->length - 1;
	gfx_plotstr(XOFFSET+3 + (pat->cursor << 1),
			YOFFSET + 8, "/\\",
			theme_cur->drum_dc);
}

void drump_drawpatname(u8 index)
{
	static const char *s1 = "ffffffffFFFFFFFF";
	static const char *s2 = "1234567812345678";
	gfx_plot(XOFFSET+7, 1, s1[index], theme_cur->drum_dc);
	gfx_plot(XOFFSET+8, 1, s2[index], theme_cur->drum_dc);
}

void drump_repeat(struct pattern *pat)
{
	static u8 n;
	static u8 i;

	n = pat->cursor;

	for (i = 0; i < n; i++) {
		if (n + i >= pat->length) break;

		pat->steps[n + i] = pat->steps[i];
		togglesdraw(pat, n + i);
		drump_inccursor(pat);
	}
}

void drump_clearfield(struct pattern *pat, u8 field)
{
	pat->steps[pat->cursor] &= ~(1 << field);
	toggledraw(pat, field, pat->cursor);
	drump_inccursor(pat);
}

void drump_editordraw(struct pattern *pat)
{
	gfx_plotstr(XOFFSET+1, YOFFSET, "BD\nSD\nCH\nOH\nLT\nHT\nCY\nAC",
			theme_cur->drum_dc);
	gfx_plotstr(XOFFSET+3, 1, "pat:", theme_cur->drum_dc);
	drump_draw(pat);
}

void drump_setmute(struct fm_drumgates *m)
{
	gfx_plotstr(XOFFSET+1, YOFFSET+0, "BD", m->bd ? theme_cur->drum_cd : theme_cur->drum_dc);
	gfx_plotstr(XOFFSET+1, YOFFSET+1, "SD", m->sd ? theme_cur->drum_cd : theme_cur->drum_dc);
	gfx_plotstr(XOFFSET+1, YOFFSET+2, "CH", m->hh ? theme_cur->drum_cd : theme_cur->drum_dc);
	gfx_plotstr(XOFFSET+1, YOFFSET+3, "OH", m->hh ? theme_cur->drum_cd : theme_cur->drum_dc);
	gfx_plotstr(XOFFSET+1, YOFFSET+4, "LT", m->tt ? theme_cur->drum_cd : theme_cur->drum_dc);
	gfx_plotstr(XOFFSET+1, YOFFSET+5, "HT", m->tt ? theme_cur->drum_cd : theme_cur->drum_dc);
	gfx_plotstr(XOFFSET+1, YOFFSET+6, "CY", m->cy ? theme_cur->drum_cd : theme_cur->drum_dc);
}
