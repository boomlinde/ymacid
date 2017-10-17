#include "drum_seq.h"
#include "fm.h"
#include "pattern.h"
#include "drumpat.h"

void dseq_init(
		struct dseq_state *s,
		struct fm_drums *d,
		struct pattern *p,
		struct fm_drumgates *m,
		u8 *shuffle)
{
	s->playing = 0;
	s->step = 0;
	s->tick = 0;
	s->mutes = m;
	s->drums = d;
	s->current = p;
	s->shuffle = shuffle;
	s->shuffled = 0;
	s->trig = 0;
	s->next = p;
}

void dseq_startstop(struct dseq_state *s)
{
	s->current = s->next;
	if (s->playing) {
		s->playing = 0;
		s->step = 0;
		s->tick = 0;
		s->shuffled = 0;
		s->trig = 0;
		fm_drumflush(s->drums);
	} else {
		s->playing = 1;
		s->trig = 1;
	}
}

void dseq_tick(struct dseq_state *s)
{
#define EN(_v, _x) (!s->mutes->_v && (current_step & (1 << _x)))
#define S (*s->shuffle)
	static u8 current_step;

	if (!s->playing) return;

	if (s->trig) {
		s->trig = 0;
		current_step = s->current->steps[s->step];
		/* Disable any of the triggered drums that may already be playing */

		if (EN(bd, BDBIT)) s->drums->set.bd = 0;
		if (EN(sd, SDBIT)) s->drums->set.sd = 0;
		if (EN(hh, CHBIT)) s->drums->set.hh = 0;
		if (EN(hh, OHBIT)) s->drums->set.hh = 0;
		if (EN(tt, LTBIT)) s->drums->set.tt = 0;
		if (EN(tt, HTBIT)) s->drums->set.tt = 0;
		if (EN(cy, CYBIT)) s->drums->set.cy = 0;
		fm_drumflush(s->drums);

		/* Enable the triggered drum */
		if (EN(bd, BDBIT)) s->drums->set.bd = 1;
		if (EN(sd, SDBIT)) s->drums->set.sd = 1;
		if (EN(hh, CHBIT)) {
			s->drums->set.hh = 1;
			s->drums->hh_sd.set.ops[0].oh = 0;
		}
		if (EN(hh, OHBIT)) {
			s->drums->set.hh = 1;
			s->drums->hh_sd.set.ops[0].oh = 1;
		}
		if (EN(tt, LTBIT)) {
			s->drums->set.tt = 1;
			s->drums->tt_cy.set.ops[0].ht = 0;
		}
		if (EN(tt, HTBIT)) {
			s->drums->set.tt = 1;
			s->drums->tt_cy.set.ops[0].ht = 1;
		}
		if (EN(cy, CYBIT)) s->drums->set.cy = 1;

		if (current_step & (1 << ACBIT)) s->drums->accent = 1;
		else s->drums->accent = 0;
		fm_drumflush(s->drums);
	}

	s->tick++;
	if (s->tick == 12) {
		s->step++;
		s->trig = 1;
		s->tick = 0;
		s->shuffled = 0;
	} else if (s->tick >= S && !s->shuffled) {
		s->shuffled = 1;
		s->step++;
		s->trig = 1;
	}

	if (s->step >= s->current->length) {
		s->current = s->next;
		s->step = 0;
	}
}
