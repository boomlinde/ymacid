#ifndef _DRUM_SEQ_H_
#define _DRUM_SEQ_H_

#include "types.h"
#include "fm.h"
#include "pattern.h"

struct dseq_state {
	u8 playing;
	u8 step;
	u8 tick;
	u8 *shuffle;
	u8 shuffled;
	u8 trig;
	struct fm_drumgates *mutes;
	struct fm_drums *drums;
	struct pattern *current;
	struct pattern *next;
};

void dseq_init(
		struct dseq_state *s,
		struct fm_drums *d,
		struct pattern *p,
		struct fm_drumgates *m,
		u8 *shuffle);
void dseq_startstop(struct dseq_state *s);
void dseq_tick(struct dseq_state *s);

#endif /* _DRUM_SEQ_H_ */
