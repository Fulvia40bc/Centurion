#ifndef END_H
#define END_H

/* Module END of docs/M6_PLAN.md (end.c): the end of the game. Final Civilization Standings 0x14294 (called by loop.c
   at 0x2309E, in f22F3C after a conquest, and at 0x5522C after the AST win), the scrolling credits 0x1330C (also
   main's, after the intro at 0x1101D), the two palette helpers 0x3CDF0 / 0x3CE44 the credits use, and the font release
   0x39F50 (main 0x118D3 after the game loop, and two PBEM routines). 0x7762C (thousands commas) is formulas.c's.

   loop.c still stops at 0x14294 through UNPORTED(0x14294u) at 0x2309E and 0x5522C: this header exports the real
   f14294 (Claude links it when the modules are merged; this module does not touch loop.c).

   Conventions: those of ui.h, game.h and kit.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx,
   ebx, ecx, then the stack; void unless a caller reads eax). D = read in the code, G = inference.

   Globals (D):
     int32 [0xF40EC] the players in play (rows drawn); int32 [0xF628C] the /C continuous test (0 normally); byte
       [0xF625D] music on, int32 [0xF6284] the sequence handle; dialog 0xBB800 (CONTINUE, id 0x2A: relabelled "EXIT"
       and moved); the /C log: int32 [0xBC8DC] the game number, [0xBC8E0] / [0xF6290] the cycle, [0xF605C] / [0xF6060]
       memory figures, [0x112A8C] / [0x112A90] the stacks' high-water marks, [0x109E04] RouteList, [0x10818C]
       RoutingMax, 0xF5500 the number text (f7762C)
     the credits table 0xC7EFC + 5r {int8 type 0 line / 1 heading (underlined) / 2 title, char *text} with its count
       int32 [0xC8038] (63); char *[0xC19D0] "message.gif" from ACIV.CT2 ([0xF40D8]); int32 [0xBB3F4] the wait divisor
       (1, 10 with /C); bytes 0xE1634 / 0xE1635 fg_intkey's key and aux
     palettes: 0xF8B40[768] the target palette (saved / loaded), 0xF5D58[768] the palette set (6-bit DAC values) */

void f14294(void);         /* D: Final Civilization Standings: nations 1..9 in play by score (f147C4, bubble sort,
                              descending, ties keep their order); unless the /C test: background 0x17, the title, per
                              row two tokens, the name underlined and the score; CONTINUE relabelled "EXIT" and centred
                              under the rows; music gcivil2; wait for EXIT; mouse hidden; the credits f1330C. With /C:
                              the statistics appended to ctest.txt, then START NEW GAME (f3FB94(0x51)). Callers: loop.c
                              0x2309E (f22F3C), 0x5522C; eax unused. BUG(1995) fixed unless g_original: the EXIT label
                              stays on the shared CONTINUE button (end.c) */
void f1330C(void);         /* D: the credits: palette black (f3CDF0), message.gif into the background buffer (byte
                              [0xF52D0] = 0x15) and on screen, pal.pal (f3CE44); then for k = 0 .. count - 11 the 11
                              lines k..k+10 drawn into the strip (0x50..0x22F, 0x5A..0x1A3) of the back buffer and
                              scrolled up 30 pixels one by one (fg_vbpaste), 40 ms apart (f78A20 in the port); any key
                              (fg_intkey) ends it. Callers: main 0x1101D, f14294 0x1464D */
void f3CDF0(void);         /* D: fg_getdacs(0, 256, 0xF8B40); memset(0xF5D58, 0, 768); fg_setdacs(0, 256, 0xF5D58) */
void f3CE44(const char *name);   /* D: if name is not empty, fg_gifpal(name, 0xF8B40); 0xF8B40 copied to 0xF5D58;
                              fg_setdacs(0, 256, 0xF5D58). One caller (0x133C6, "pal.pal") */
void f39F50(int i);        /* D: release font i (Fastgraph/Fonts 0x8E0F8 with int16 0xF8B2C[i]), all fonts for i == -1.
                              The port's fonts stay loaded: the release is a no-op (end.c). Callers: main 0x118D3
                              (-1), PBEM 0x5055C / 0x50D08 */

#endif
