#ifndef _EDIT_H_
#define _EDIT_H_

#include "fm.h"
#include "types.h"

void edit_drawsynth(struct fm_voice *fm, int key);
void edit_updatemouse(struct fm_voice *fm);
void edit_drawdrums(struct fm_drums *d, int key);
void edit_updatemouse(struct fm_voice *fm);

#endif /* _EDIT_H_ */
