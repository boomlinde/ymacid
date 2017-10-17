#ifndef _PATTERN_H_
#define _PATTERN_H_

#define MAXLEN 16

struct pattern {
	u8 cursor;
	u8 length;
	u8 steps[MAXLEN];
};

#endif /* _PATTERN_H_ */
