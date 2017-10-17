#ifndef _DRUMPAT_H_
#define _DRUMPAT_H_

#include "types.h"
#include "pattern.h"
#include "fm.h"

#define BDBIT 0
#define SDBIT 1
#define CHBIT 2
#define OHBIT 3
#define LTBIT 4
#define HTBIT 5
#define CYBIT 6
#define ACBIT 7

void drump_toggle(struct pattern *pat, u8 field);
void drump_inclength(struct pattern *pat);
void drump_declength(struct pattern *pat);
void drump_editordraw(struct pattern *pat);
void drump_draw(struct pattern *pat);
void drump_inccursor(struct pattern *pat);
void drump_deccursor(struct pattern *pat);
void drump_clearstep(struct pattern *pat);
void drump_rotleft(struct pattern *pat);
void drump_rotright(struct pattern *pat);
void drump_clearcursor(struct pattern *pat);
void drump_setcursor(struct pattern *pat, u8 index);
void drump_drawpatname(u8 index);
void drump_repeat(struct pattern *pat);
void drump_clearfield(struct pattern *pat, u8 field);
void drump_setmute(struct fm_drumgates *m);

#endif /* _DRUMPAT_H_ */
