/* Module ANIM of docs/M6_PLAN.md: the entry points 0x7A4A0 and 0x7A4C4 (literal translations; the address of each
   instruction a line comes from is in its comment). The player 0x7A0A0 is not ported yet (needs fg_showflic, handoff
   Task 17): with animations on the port stops at not_ported(0x7A0A0), as kit.c's f79270 does. */

#include <stdint.h>

#include "anim.h"
#include "game.h"
#include "state.h"

static void f7A0A0(int32_t anim)                      /* ANIM (FLC player): not ported yet */
{
    (void)anim;
    not_ported(0x7A0A0u);
}

/* 0x7A4A0 */
void f7A4A0(void)
{
    setD32(0xCAC60u, 0xFFFFFFFFu);                    /* 0x7A4B3 */
    setD32(0xCAC64u, 0xFFFFFFFFu);                    /* 0x7A4B9 */
}

/* 0x7A4C4 */
void f7A4C4(int anim)
{
    (void)anim;                                       /* 0x7A4D4 [ebp-4] = eax, never read */
    if ((D8(0xF097Cu) & 0x04) != 0)                   /* 0x7A4D7 test byte [0xF097C], 4; je */
        f7A0A0(S32(0xF783Cu));                        /* 0x7A4E5 */
}
