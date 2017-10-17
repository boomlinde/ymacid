#ifndef _THEME_H_
#define _THEME_H_

#include "types.h"

struct theme {
	u8 kb_black;
	u8 kb_white;

	u8 bass_set;
	u8 bass_dc;
	u8 bass_step;
	u8 bass_hlstep;

	u8 drum_dc;
	u8 drum_set;
	u8 drum_step;
	u8 drum_hlstep;
	u8 drum_cd;

	u8 bass_ed;
	u8 bass_hled;
	u8 drum_ed;
	u8 drum_hled;
};

extern struct theme *theme_cur;
extern struct theme theme_data;
extern struct theme theme_acid;

#endif /* _THEME_H_ */
