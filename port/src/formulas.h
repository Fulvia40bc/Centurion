#ifndef FORMULAS_H
#define FORMULAS_H

/* The small pure formulas of the game loop (docs/M6_PLAN.md "Mistral candidates" item 5, TASK 15): fifteen
   functions that read only data-object bytes (0x7762C writes into the caller's buffer) and call no screen,
   input or game_rand routine. Prototypes in the original's register-argument order (eax, edx); see formulas.c
   for each address. */

int  f1B4D4(int n);         /* 0x1B4D4: 1 if the stock (rec+0x99) < cities * tax rate (rec+0x51 * rec+0x3F) */
int  f1B538(int n);         /* 0x1B538: 1 if n holds exactly a full set of commodities 1..18 (vs the table 0xBD541..) */
int  f1C2DC(int n);         /* 0x1C2DC: the token shortfall: tax + need (tokens in n's areas) - stock, 0 if enough */
int  f193AC(void);          /* 0x193AC: value of the goods at 0xF67E4 (Mining bonus included) */
int  f19448(void);          /* 0x19448: the same for nation record 0 (0xF14CC) */
int  f176BC(int n, int k);  /* 0x176BC: cost of advance slot k of n, capped at the credit already earned */
int  f2AC94(int n, int x);  /* 0x2AC94: what is left after removing x unit points (cities count 5) */
int  f6B928(int a, int b);  /* 0x6B928: score difference of nations a, b in the current round / 50 */
int  f7CAB4(int n);         /* 0x7CAB4: 1 if none of n's 9 purchase slots (0xF52E0 + 40n + 4j, j 1..9) is negative */
int  f6DB00(int n);         /* 0x6DB00: whether n can buy a card (treasury rec+0x3E vs the stacks) */
void f7762C(int v, char *buf);  /* 0x7762C: v with thousands commas into buf (callers pass 0xF5500) */
int  f46A4C(int n);         /* 0x46A4C: tokens over the population limits where n is alone */
int  f46B34(int n);         /* 0x46B34: 1 if the population limits of n's areas cover the token supply 0xF4898 */
int  f690AC(int c, int n);  /* 0x690AC: the harm calamity card c would do to n */
int  f667B0(int n);         /* 0x667B0: the computer's tax rate for n (1..3) */

#endif
