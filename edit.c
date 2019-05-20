#include "edit.h"
#include "gfx.h"
#include "fm.h"
#include "theme.h"
#include "mouse.h"

static char *hlb;
static char *hld;
static char **hlp;
static int k;
static u8 originx;
static u8 vx;
static u8 vy;

static u8 col1;
static u8 col2;
static u8 mouse_max;
static i16 mouse_state;

void edit_updatemouse(struct fm_voice *fm)
{
	static i16 x;
	static i16 new;
	if (!mouse_enabled) return;
	if (!hlb) return;

	x = 0;
	mouse_motion(0, &x);
	mouse_state -= x << 1;

	new = mouse_state >> 8;

	if (new < 0) {
		new = 0;
		mouse_state = 0;
	} else if (new > mouse_max) {
		new = mouse_max;
		mouse_state = new << 8;
	}

	if (new != *hlb) {
		*hlb = new;
		if (hlp == &hlb) edit_drawsynth(fm, 0);
	}
}

static void control(int key, char *name, u8 max, u8 *v)
{
	static u8 colors;
	static i16 value;

	if (k == key) {
		*hlp = v;

		/* handle mouse only for bassline */
		if (hlp == &hlb) {
			mouse_max = max;
			mouse_state = (i16)*v << 8;
		}
	}

	value = *v;

	if (v == *hlp) {
		colors = col1;
		if (k == ',') value--;
		else if (k == '.') value++;
		else if (k == '<') value-=4;
		else if (k == '>') value+=4;

		if (value < 0) value = 0;
		else if (value > max) value = max;

		/* Update mouse state if bassline control */
		if (*v != value && hlp == &hlb) {
			mouse_state = (i16)*v << 8;
		}

		*v = value;
	} else {
		colors = col2;
	}

	gfx_plotstrf(vx, vy, "%s:%02x", colors, name, *v);
	vx += strlen(name) + 4;
}

static void label(char *s)
{
	gfx_plotstr(vx, vy, s, DC);
	vx += strlen(s) + 2;
}

static void setorigin(u8 x, u8 y)
{
	vx = x;
	originx = x;
	vy = y;
}

static void linebreak(void)
{
	vx = originx;
	vy += 2;
}

void edit_drawsynth(struct fm_voice *fm, int key)
{
#define MOD &fm->set.ops[0]
#define CAR &fm->set.ops[1]
	col1 = theme_cur->bass_hled;
	col2 = theme_cur->bass_ed;

	hlp = &hlb;

	k = key;
	setorigin(2, 21);

	if (hlb == 0) {
		hlb = MOD.level;
		mouse_max = 0x3f;
		mouse_state = (u16)*(MOD.level) << 8;
	}

	control('z', "ML",            0x3f, MOD.level);
	control('x', "MD",             0xf, MOD.decay);
	control('c', "MM",             0xf, MOD.multiplication);
	control('v', "MW", fm_opl3 ? 7 : 3, MOD.waveform);
	control('b', "MS",             0xf, MOD.sustain);
	control('n', "FB",             0x7, &fm->set.feedback);
	linebreak();
	control('Z', "CL",            0x3f, CAR.level);
	control('X', "CD",             0xf, CAR.decay);
	control('C', "CM",             0xf, CAR.multiplication);
	control('V', "CW", fm_opl3 ? 7 : 3, CAR.waveform);
	control('B', "AL",            0x3f, MOD.accent_level);
#undef MOD
#undef CAR
}

void edit_drawdrums(struct fm_drums *d, int key)
{
#define BDMOD &d->bd.set.ops[0]
#define BDCAR &d->bd.set.ops[1]
#define HH &d->hh_sd.set.ops[0]
#define SD &d->hh_sd.set.ops[1]
#define TT &d->tt_cy.set.ops[0]
#define CY &d->tt_cy.set.ops[1]

	col1 = theme_cur->drum_hled;
	col2 = theme_cur->drum_ed;

	hlp = &hld;
	k = key;
	setorigin(2, 17);

	if (hld == 0) hld = BDMOD.level;

	control('q', "BC", 0x3f, BDMOD.level);
	control('w', "BP", 24, &d->bd.set.pitch_offset);
	control('e', "BL", 0x3f, BDCAR.level);
	control('r', "BD",  0xf, BDCAR.decay);
	control('t', "BT",  0xf, BDMOD.decay);
	control('y', "BM",  0xf, BDMOD.multiplication);
	linebreak();
	control('a', "HL", 0x3f, HH.level);
	control('s', "HD",  0xf, HH.decay);
	control('d', "SL", 0x3f, SD.level);
	control('f', "SD",  0xf, SD.decay);
	control('g', "SP", 24, &d->hh_sd.set.pitch_offset);
	linebreak();
	control('z', "TL", 0x3f, TT.level);
	control('x', "TD",  0xf, TT.decay);
	control('c', "TM",  0xf, TT.multiplication);
	control('v', "CL", 0x3f, CY.level);
	control('b', "CD",  0xf, CY.decay);
	control('n', "TP", 24, &d->tt_cy.set.pitch_offset);
#undef BDMOD
#undef BDCAR
#undef HH
#undef SD
#undef TT
#undef CY
}
