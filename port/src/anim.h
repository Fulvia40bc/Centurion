#ifndef ANIM_H
#define ANIM_H

/* Module ANIM of docs/M6_PLAN.md (anim.c): the calamity animation 0x7A0A0 (its FLC from ACIV.CT4 through fg_showflic,
   its WAV, the kaboom3 counter in aciv.cfg and its game_rand roll), Zeus 0x79ADC, the BRPC picture 0x79C70 and the two
   small entry points the calamity screens call. Conventions: those of game.h (fXXXXX = the original at 0xXXXXX,
   Watcom register arguments eax, edx; no caller reads eax: every call site checked). D = read in the code, G =
   inference. */

void f7A0A0(int id);       /* D: the calamity animation and sound for id (0x12..0x1E, Earthquake .. Piracy; the callers
                              pass [0xF783C]); nothing if id == [0xCAC64] (already shown), else [0xCAC64] = [0xCAC60] =
                              id. The sound: 0xCAA38[id], or 0xCAA6C[id] ("kaboom3.wav", Earthquake and Volcano only)
                              when the 6-bit counter in bits 9..14 of word [0xF0980] is 0 and game_rand * 10 >> 15 == 0
                              (one game_rand call whenever the counter is 0); the counter is then set to 51, else a
                              non-zero counter counts down by one; aciv.cfg (6 bytes from 0xF097C) is rewritten whenever
                              the counter changes. With animations (bit 0x04 of [0xF097C]) the FLC 0xCAAA0[id] plays
                              centred in the map window, the area under it saved to and restored from "junk2". Callers:
                              f79270 (0x79465) and f7A4C4 (0x7A4E5), both only with animations on */
void f79ADC(const char *name, const char *wav);   /* D: an FLC of ACIV.CT4 ([0xF40E0]) centred in the map window with
                              the WAV wav (sample slot 0), the area saved to / restored from "junk2". One caller: f79270
                              (0x79499: "zeus.flc", "zeus.wav", animations on) */
void f79C70(const char *gif, const char *wav);    /* D: a GIF of ACIV.CT1 ([0xF40D4]) centred in the map window: drawn
                              into the back buffer [0xF52C8], pasted to the screen, the back buffer then restored from
                              "junk2"; the WAV wav (slot 0) loaded, and started if not empty. One caller: f79270
                              (0x794BE with animations on, 0x794E9 with them off: "brpc.gif", "") */
void f7A4A0(void);         /* D: [0xCAC60] = [0xCAC64] = -1 (0x7A4AE..0x7A4B9): the next f7A0A0 plays again */
void f7A4C4(int anim);     /* D: if bit 0x04 of [0xF097C] (animations on): f7A0A0([0xF783C]), the current calamity's
                              animation; the argument (eax, -600 or 2000 at the call sites) is stored and never read */

#endif
