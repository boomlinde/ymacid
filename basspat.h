#ifndef _BASSPAT_H_
#define _BASSPAT_H_

#include "types.h"

#define MAXLEN 16

#define ACCENT  7
#define SLIDE   6
#define OCTMIN  5
#define OCTPLUS 4

struct pattern {
	u8 length;
	u8 steps[MAXLEN];
};

u8 get_note(u8 step);
u8 next_index(struct pattern *pat, u8 index);
void bass_setnote(struct pattern *pat, u8 note, u8 index);
void bass_toggle(struct pattern *pat, u8 field, u8 index);
void bass_inclength(struct pattern *pat);
void bass_declength(struct pattern *pat);
void bass_randomize(struct pattern *pat, u8 index);
void bass_editordraw(struct pattern *pat);
void bass_draw(struct pattern *pat);

#endif /* _BASSPAT_H_ */
