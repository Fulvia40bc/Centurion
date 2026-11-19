#ifndef ANIM_H
#define ANIM_H

/* Module ANIM of docs/M6_PLAN.md (anim.c), the part ported so far: the two small entry points the calamity screens
   call. The animation player itself (0x7A0A0, the calamity FLC from ACIV.CT4 through fg_showflic; 0x79ADC Zeus;
   0x79C70 the BRPC picture) waits for fg_showflic (handoff Task 17): with animations on, f7A4C4 stops at
   not_ported(0x7A0A0). Conventions: those of game.h. D = read in the code. */

void f7A4A0(void);         /* D: [0xCAC60] = [0xCAC64] = -1 (0x7A4AE..0x7A4B9; the animation player's state, G) */
void f7A4C4(int anim);     /* D: if bit 0x04 of [0xF097C] (animations on): f7A0A0([0xF783C]), the current calamity's
                              animation; the argument (eax, -600 or 2000 at the call sites) is stored and never read */

#endif
