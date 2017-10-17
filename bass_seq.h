#ifndef _BASS_SEQ_H_
#define _BASS_SEQ_H_

#include "types.h"
#include "fm.h"
#include "basspat.h"

struct bseq_state {
	u8 playing;
	u8 step;
	u8 tick;
	u8 last_step;
	u8 trig;
	u8 *shuffle;
	u8 *doshuffle;
	u8 shuffled;
	int *tune;
	struct fm_voice *voice;
	struct pattern *current;
	struct pattern *next;
	float slide_rate;
};

void bseq_init(
		struct bseq_state *s,
		struct fm_voice *v,
		struct pattern *p,
		int *t,
		u8 *shuffle,
		u8 *doshuffle);
void bseq_startstop(struct bseq_state *s);
void bseq_tick(struct bseq_state *s);

#endif /* _BASS_SEQ_H_ */
