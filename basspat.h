#ifndef _BASSPAT_H_
#define _BASSPAT_H_

#include "types.h"
#include "pattern.h"

#define ACCENT  7
#define SLIDE   6
#define OCTMIN  5
#define OCTPLUS 4
#define NOTE 0xf

float basspat_getnote(u8 step);
void bass_setnote(struct pattern *pat, u8 note);
void bass_toggle(struct pattern *pat, u8 field);
void bass_inclength(struct pattern *pat);
void bass_declength(struct pattern *pat);
void bass_randomize(struct pattern *pat);
void bass_editordraw(struct pattern *pat);
void bass_draw(struct pattern *pat);
void bass_inccursor(struct pattern *pat);
void bass_deccursor(struct pattern *pat);
void bass_clearstep(struct pattern *pat);
void bass_rotleft(struct pattern *pat);
void bass_rotright(struct pattern *pat);
void bass_clearcursor(struct pattern *pat);
void bass_setcursor(struct pattern *pat, u8 index);
void bass_drawpatname(u8 index);
void bass_incnote(struct pattern *pat);
void bass_decnote(struct pattern *pat);
void bass_repeat(struct pattern *pat);
void bass_incnotes(struct pattern *pat);
void bass_decnotes(struct pattern *pat);
void bass_clearfield(struct pattern *pat, u8 field);

#endif /* _BASSPAT_H_ */
