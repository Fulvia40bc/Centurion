# Text sweep (Handoff Task 3)

Read all 2,275 lines of `docs/strings.txt` (game strings from 0x000B0058 to about 0x000CAC76; the Miles AIL /
Watcom / Fastgraph library strings from about 0x000B9AAB were skipped per the file header). Addresses are linear
(base 0xB0000), as in `strings.txt`. Slips are proposed to be fixed in the port by default; `--original` keeps
them. The two known ones from docs/OVERVIEW.md §6 are confirmed: "Montheism" (x2) and "popped once to often"
(actually x4, see below).

## Slips

One line per slip: address — the text — proposed fix — why.

- 0x000B0292 — "you need to free up and additional %d bytes of XMS memory" — "an additional" — grammar (a/an); the same string also has a double space in "sound effects,  you" -> single space
- 0x000B1A05 — "%s has no cities to be affected by volcanos or earthquakes." — "volcanoes" — spelling; contradicts "volcano" in 0x000B1A41
- 0x000B1EEA — "%s converts %s by Montheism!" — "Monotheism" — spelling; contradicts 0x000B1E9F and the advance's name at 0x000B17E2 / 0x000B6D83
- 0x000B1F07 — "%s converts %s by Montheism!" — "Monotheism" — same as above (second copy)
- 0x000B222C — "Enlightenment nullifys the effect." — "nullifies" — spelling
- 0x000B2FB3 — "%s, you are a beneficary of Civil War." — "beneficiary" — spelling; contradicts 0x000B2DBD which has it right
- 0x000B2FF2 — "%s is a beneficary of Civil War, but cannot fully absorb..." — "beneficiary" — spelling; same contradiction
- 0x000B308C — "%s, you are a beneficary of Civil War. Choose tokens to annex." — "beneficiary" — spelling
- 0x000B30CB — "%s is a beneficary of Civil War, but cannot fully absorb the Second Faction tokens..." — "beneficiary" — spelling
- 0x000B35A7 — "%s choses the %s to flood." — "chooses" — spelling
- 0x000B5AE4 — "ship maint./const.for the other players are not shown" — "maint./const. for" — missing space after the period
- 0x000B5CDE — "%s,you have no stock to pillage with." — "%s, you ..." — missing space after the comma (the only game string with this defect)
- 0x000B5D16 — "%s, choosing number units to transfer from stock to treasury." — "choosing number of units" — missing word
- 0x000B76DB — "20 point Science credit" — "Sciences credit" — contradicts the category "Sciences." at 0x000B76BA and the same line at 0x000B77BA / 0x000B78B6 / 0x000B7996
- 0x000B81C9 — "reduces the effects of Iconoclasm and Heresey." — "Heresy" — spelling; contradicts "Iconoclasm & Heresy" at 0x000B65AD / 0x000B437F / 0x000B992A
- 0x000B854B — "provided the viction does not hold Monotheism or Theology." — "victim" — spelling
- 0x000B8904 — "Democrary spares %s from a Tax Revolt." — "Democracy" — spelling; contradicts 0x000B17A6 / 0x000B6D47
- 0x000B9679 — "World State Stack has popped once to often." — "too often" — spelling (known)
- 0x000B96A5 — "World State Stack has popped once to often." — "too often" — spelling (known)
- 0x000B96D1 — "Move Stack has popped once to often." — "too often" — spelling (known)
- 0x000B96F6 — "Move Stack has popped once to often." — "too often" — spelling (known)
- 0x000C7CA8 — "One or more nations drawing earlier has the possibility of buying a card..." — "have the possibility" — subject-verb agreement ("nations ... has")
- 0x000B280A — "%s, select cities to be received thru Tax Revolt." — "through" — spelling (informal "thru")
- 0x000B25EA — "no coastal cities for the Pirates to takeover." — "to take over" — grammar (verb must be two words here)
- 0x000B2705 — "Secondary victims have no coastal cities for the Pirates to takeover." — "to take over" — grammar
- 0x000B33BB — "%s, Choose side to play." — "%s, choose side to play." — capitalization after the comma
- 0x000B1E6D — "Crete can not be the Victim of Barbarian Hordes." — "cannot ... the victim" — spelling/capitalization ("can not" -> "cannot", mid-sentence "Victim" -> "victim")
- 0x000B6EB4, 0x000B6FA0, 0x000B7074, 0x000B7167, 0x000B729E, 0x000B7424, 0x000B7550, 0x000B76DB, 0x000B77BA, 0x000B78B6, 0x000B7996, 0x000B7A82, 0x000B7B81, 0x000B7C93, 0x000B7DAC, 0x000B824E, 0x000B8319, 0x000B83E0 — "N point X credit" (N = 5/10/15/20) — "N points X credit" (or "N-point" as an adjective) — grammar: singular "point" after a plural number; each of these lines itself uses "10 points to Democracy..." right after, so the same line contradicts the fix. Systematic (18 help texts) — flagged for Erik, default is to fix.
- 0x000B7217 — "no direct effects - city reduction is mitigated by one token." — "No direct effects - ..." — style: lowercase sentence start

## Jokes and odd lines worth Erik's attention

- 0x000B2554 — Piracy with no room left: "The management of Skull & Crossbones, Inc. regret to inform you that they are unable to add new cities to their portfolio.  %s is spared from Piracy." (OVERVIEW §6.3)
- 0x000B1855 — "The BRPC is deadlocked. They decide to settle in %s." and 0x000B188A — "%s has been wiped out by the Barbarians. Satisfied with a job well done they settle in %s." — the Barbarian Ravaging and Plundering Committee (OVERVIEW §6.4)
- 0x000B19CD — "Zeus takes pity upon %s and discards ..." with its own picture/animation/sound (OVERVIEW §6.5)
- "junk" / "junk2" placeholder strings: 0x000B160C, 0x000B161A, 0x000B161F, 0x000B1681, 0x000B1686, 0x000B1695, 0x000B400C, 0x000B4011, 0x000B4016, 0x000B4049, 0x000B4075, 0x000B4083, 0x000B4580, 0x000B4585, 0x000B5327, 0x000B537F, 0x000B58AE, 0x000B8FC3, 0x000B8FE4, 0x000B90C3, 0x000B90D1, 0x000B935F, 0x000B9364, 0x000B957D, 0x000B9589, 0x000B958E, 0x000B959C, 0x000B95A1, 0x000B95A8, 0x000B95AE, 0x000B95B4, 0x000B9602, 0x000B9614
- Developer memory statistics (OVERVIEW §6.7): 0x000B0F01 "Memory available after game is loaded: %s", 0x000B0F2C "Max memory dynamically allocated by arrays: %s", 0x000B0F5C "Max Memory used including .map analysis: %s", 0x000B0F89 "Memory Headroom: %s", 0x000B0F9E "Max WorldStack: %d   Max MoveStack: %d  Max RouteList: %d", 0x000B0FD9 "RoutingMax: %d"; stack errors 0x000B961D / 0x000B964C "... Stack has blown its max size. > %d" plus the four "popped once to often" above
- Routine-name traces that will name functions in the disassembly: "NationLosesUnits" (0x000B2C28), "ChooseCitiesToLoseToPirates" (0x000B27DF), "ChooseFirstFaction" (0x000B32D8), "DrawTradingCards" (0x000B8BF5), "DrawInitialDrawingScreen" (0x000B8C0E), "ManualPopExpansion" (0x000B8ABC), "PBEMViewer" (0x000B5C23), and "DoShipContruction" (0x000B623C, 0x000B6257, 0x000B62D9, 0x000B62F4) — misspelled in the binary itself
- 0x000B6358 / 0x000B637E / 0x000B63A4 — "%s considers building ships. (%d,?,?)" / "(0,%d,%d)" — "?,?" placeholders left in
- 0x000B074D / 0x000B0760 — "Ctest.txt erased." / "Ctest.txt will be appended to." vs lower-case "ctest.txt" at 0x000B0743 / 0x000B06FF — harmless on DOS (case-insensitive), inconsistent
- 0x000B45B6 — "Round %d score may not accurately reflect the present situation." — an honest note in the scoring history
- 0x000B4C14 etc. — pkzip command lines with ">NUL:" (trailing colon) and "del *.pl? >NUL" — PBEM plumbing (OVERVIEW §7 question 5)
- 0x000B0949 — "requires a 640x480 256 color graphics mode. " — trailing space before \n (cosmetic only)
- Library region from ~0x000B9AAB (AIL debug traces, Watcom runtime, Fastgraph banners) — not game texts, skipped; includes its own typos like "oCould not allocate..." which are Miles/Watcom's, not ours

## Added 2026-11-18 (Claude, modules INFO / MENU1 / MENU2): help files and advance descriptions

RULED 2026-11-19 (Erik): the help texts stay as the original has them; only obvious errors are fixed (by default,
`--original` keeps them). Fixed (Erik, 19 Nov): "affect" for "effect" (SUPERSTN), "Acquistion" (BUYCIV), the
full stop after `Salt?"` (TRADWANT, removed), the space in "button , only" (TRADOFFR), "a F" -> "an F" (CIVILWAR).
Done in menu1.c's help_fix, checked on the real files (port/test/helpfix_test.c). Everything else in this section stays (less/fewer,
amount/number, its/their, capitals, wording). The City Support help is a separate matter: that is the 1995 bug of the
wrong help file, which stays fixed.

Not in the EXE's own strings, so not in the sweep above. None is changed yet: each waits for Erik's ruling.

**Help files** (`*.HLP` in ACIV.CT3, shown by HELP):
- BUYCIV: "Alternate Tool Acquistion Color toggle". The word is misspelled, and the toggle's real name is
  "Alternate Tool Acquisition Colors".
- CIVILWAR: "replaced by a F (first faction) piece" should be "an F".
- SUPERSTN: "Deism limits the affect to one city" should be "effect".
- "less" for "fewer": SHOWAST "A nation with less cities", CITYCNST "less than nine cities", PIRACY "having less than
  two coastal cities".
- "amount of Tokens" for "number of Tokens": EPIDEMIC and FAMINE "the amount of Tokens which replace it", SLAVERVT
  "the maximum amount of Tokens".
- TRADOFFR: "RESPOND TO OFFER button , only one" (space before the comma); "respond to any and all of the offers by
  clicking on its" ("its" for "their").
- TRADWANT: "Who has Grain for Salt?"." (a question mark followed by a full stop).
- MOVEMENT says "MOVE TOKENS button", then "MOVE TOKEN button" in the same text; MOVESHIP also says "MOVE TOKEN".
- VIEWMOVE (PBEM only): "Not only does this serve as a refresher course on your just completed move since the time
  which has elapsed between moves" is garbled.
- Capitals not used consistently: "population limit" (SUPERSTN, TREACHRY) vs "Population Limit"; "Scroll arrow"
  (SHIPCNST); "Tool" (EXCESS) vs "tool".
- START: "tan dots" where the other texts say "tan circle"; "Any excess are removed".
- ACQRGOOD: the chained sum "(3 x 3 = 9 x 2 = 18)".

**Advance descriptions** (Civilization Tools pop-up; in the EXE, but missed above):
- "Allow" for "Allows": Roadbuilding "Allow movement through two areas", Military "Allow the holder to build ships",
  Monotheism "Allow the conversion". The other descriptions say "Allows purchase".

**A missing help title** (needs a ruling with the fix): the City Support phases show Ship Construction's help (1995
bug, fixed by default). The help file CITYSUP.HLP exists, but the game has no title string for it. The port uses
"City Support" (the phase table spells it "CITY SUPPORT").

**Found 2026-11-19 (module RULES review), for the calamity module CAL1:** at 0x2110B the article for "replaced by
%s %s city." is chosen the wrong way round, so the game says "replaced by a African city" / "an Thracian city"
(0x21021's "A/An ... city in %s was" is right). Obvious error under the 19 Nov ruling: fixed by default when CAL1 is
translated.
