#include "bass_seq.h"
#include "fm.h"
#include "basspat.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

void bseq_init(
		struct bseq_state *s,
		struct fm_voice *v,
		struct pattern *p,
		int *t,
		u8 *shuffle,
		u8 *doshuffle)
{
	s->playing = 0;
	s->step = 0;
	s->tick = 0;
	s->tune = t;
	s->voice = v;
	s->current = p;
	s->shuffle = shuffle;
	s->doshuffle = doshuffle;
	s->shuffled = 0;
	s->trig = 0;
	s->next = p;
}

void bseq_startstop(struct bseq_state *s)
{
	s->current = s->next;
	if (s->playing) {
		s->playing = 0;
		s->last_step = 0;
		s->step = 0;
		s->tick = 0;
		s->slide_rate = 0.0;
		s->voice->set.gate = 0;
		s->trig = 0;
		s->shuffled = 0;
		fm_flush(s->voice);
	} else {
		s->playing = 1;
		s->trig = 1;
	}
}

void bseq_tick(struct bseq_state *s)
{
#define HASFIELD(field, x) ((x) & (1<<(field)))
#define HASSLIDE(x) (HASFIELD(SLIDE, x) && ((x) & NOTE) != NOTE)
#define GN basspat_getnote
#define TUNE (40.0 + (float)*s->tune)
#define S (*s->doshuffle ? *s->shuffle : 12)
#define NS (*s->doshuffle ? S - 12 : 0)

	static u8 current_step;

	if (!s->playing) return;

	current_step = s->current->steps[s->step];

	if (s->trig && (current_step & NOTE) != NOTE) {
		s->trig = 0;
		s->slide_rate = 0;
		s->voice->set.gate = 1;
		if (HASSLIDE(s->last_step)) {
			s->voice->set.pitch = TUNE + GN(s->last_step);
			s->slide_rate = (GN(current_step) - GN(s->last_step)) / 11.0;
		} else {
			s->voice->set.pitch = TUNE + GN(current_step);
			s->voice->set.ops[0].accent = HASFIELD(ACCENT, current_step) != 0;
		}
	}

	s->voice->set.pitch += s->slide_rate;
	if (s->slide_rate>0 && s->voice->set.pitch>TUNE+GN(current_step)) {
		s->voice->set.pitch = TUNE + GN(current_step);
	} else if (s->slide_rate<0 && s->voice->set.pitch<TUNE+GN(current_step)) {
		s->voice->set.pitch = TUNE + GN(current_step);
	}

	s->tick++;
	if (!HASSLIDE(current_step) && (s->tick == 6 - NS || s->tick == 18 + NS)) {
		s->voice->set.gate = 0;
	}

	if (s->tick == 24) {
		s->shuffled = 0;
		s->trig = 1;
		s->tick = 0;
		s->step++;
		s->last_step = current_step;
	} else if (s->tick >= S && !s->shuffled) {
		s->shuffled = 1;
		s->trig = 1;
		s->step++;
		s->last_step = current_step;
	}

	if (s->step >= s->current->length) {
		s->trig = 1;
		s->current = s->next;
		s->step = 0;
	}

	fm_flush(s->voice);

#undef HASFIELD
#undef HASSLIDE
#undef GN
#undef TUNE
#undef S
#undef NS
}
