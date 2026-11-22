# Fixes for the 1995 original's bugs

The 1995 DOS game (Advanced Civilization) contains bugs. This port fixes them **by default**. Running with
`--original` keeps the original behaviour wherever it can be reproduced. A few fixes apply **in both modes**: the
original read memory it never wrote (stack garbage), and that cannot be reproduced, so the port does the same sane
thing either way. Spelling and wording slips in the messages are fixed the same way, per Erik's ruling of
19 November (`docs/TEXT_SWEEP.md`); `--original` keeps the original text.

Counts from the notes in the port's source (`tools/verify/fixes.py list`): **103** fixes active by default
(fixed unless `--original`), **8** fixed in both modes, **3** text fixes.

"(probably)" in a row means the effect on the game was graded G (a guess) in the notes; the code reading is solid.

## Setup (nation choice, taxes and population at round start, saving and loading)

| What was wrong | The port does (default) | `--original` | Where |
|---|---|---|---|
| When the computer picked nations, nine places counted a taken nation *up* instead of down, so it could hand out one nation too many. | Counts down everywhere. | Keeps the original count. | `nations.c A 0x30339` |
| With 7 players, two switch cases gave the computer a nation even when no slots were left. | Those cases respect the remaining slots. | Keeps the original behaviour. | `nations.c B 0x30aae` |
| A 4-player game of Iberia, Illyria, Thrace and Crete (no Africa) gave the human Illyria the wrong nation card set (a copied test). | Uses the test that fits the card set. | Keeps the original test. | `aistart.c @15254 0x15254` |
| A rejected save file (wrong CRC or version) had already replaced the whole game state before the rejection was shown. | Checks the file first, loads only if it passes. | Loads first, then rejects (state already replaced). | `save.c 1 0x5ff28` |
| The "Restoring" message was measured in the wrong font and could run off both screen edges. | Measures in the font it prints with. | Keeps the original measuring. | `save.c 2 0x5ff28` |
| While typing a save description, the CANCEL button could not be clicked (the click test was dead code). | The mouse position is read after each key, so CANCEL works. | Keeps the dead click branch. | `save.c 3 0x60694` |
| Leaving the dialog with Esc kept a hidden back buffer as the drawing surface, so the next screen drew into it. | Closes the back buffer before leaving. | Keeps the original leak. | `save.c 4 0x60694` |
| Save files older than version 0x28 were read 22 bytes misaligned (they have no file tag). | Reads old files aligned. | Keeps the 22-byte shift. | `saveload.c A 0x601d8` |
| Save versions 0x2D/0x2E kept the previous game's random number table instead of regenerating it. | Regenerates it. | Keeps the old table. | `saveload.c B 0x112a94` |
| "Democrary spares %s from a Tax Revolt." | "Democracy spares %s ...". | Keeps the misspelling. | `tax.c 1 0xb8904` |
| During a tax revolt the player buttons highlighted the wrong nation (a stale "current player"). | Highlights the revolting nation. | Keeps the stale highlight. | `tax.c 2 0x66408` |
| When the last city was lost to a revolt, the empty choice list showed the name "PIRACY" and corrupted a memory handle. | Shows no name and no highlight while the list is empty. | Keeps the original list handling. | `tax.c 3 0x66ff8` |
| Population expansion could pass over the list forever when tokens were left but none were allowed. | The pass ends. | Keeps the endless pass. | `pop.c @0 0x000000` |
| "Distribute all" used an uninitialised counter. | Starts the counter at 0. | Reads the stack garbage as the original did (not reproducible). | `pop.c @0 0x000000` |

## Trade (offers, counter-offers, buying goods)

| What was wrong | The port does (default) | `--original` | Where |
|---|---|---|---|
| A complete offer with a bluff appended its warning to uninitialised memory. | Starts the message from empty text (both modes: the garbage cannot be reproduced). | Same text as default in the port; the original printed garbage. | `trade.c 1 0x71768` |
| A counter-offer's "wanted" count was stored from uninitialised memory (probably). | Stores the offerer's own count. | Stores 0. | `trade.c 2 0x71768` |
| Clicking the two lowest pixel rows of the card chart could pick a calamity card as a bluff or wanted card (probably). | Ignores clicks outside the chart. | Keeps the original hit test. | `trade.c 3 0x7109c` |
| Removing a calamity from a hand could shift the list from an uninitialised index (probably, unreachable). | Keeps the record unchanged. | Starts the shift at index 0. | `trade.c 4 0x70984` |
| If the offerer had no public record, the trade log shifted from an uninitialised index (probably, unreachable). | Writes no cards for the offerer. | Starts at index 0. | `trade.c 5 0x75074` |
| "The responses to African offer..." missed the article. | "The responses to the African offer...". | Keeps the original wording. | `trade.c 6 0xb92a3` |
| Clicking between the stack columns bought "stack -100": 58 gold paid for nothing. | Ignores the click. | Keeps the original payment. | `cards.c 1 0x6d8e8` |
| A stack the treasury could not afford could still be bought, driving the treasury below 0. | The hit test also checks the treasury. | Keeps the original hit test. | `cards.c 2 0x6d8a7` |
| CANCEL after starting a purchase left the purchase counted (probably only visible in saves). | CANCEL takes the count back. | Keeps the counted purchase. | `cards.c 3 0x6d800` |
| Without the variant rule, BUY took the top card of an empty stack unchecked: paid 18 gold, got nothing (probably). | Buys, pays and counts nothing. | Keeps the original behaviour. | `cards.c 4 0x6d5f3` |
| The computer's "Buy" counter showed 0 after a bought advance when its last try did not buy (probably only the screen). | Counts the purchases. | Keeps the original counter. | `cards.c 5 0x6c6c1` |
| With more than 8 cards and no commodity left, the discard screen waited forever (probably). | The loop ends when only calamities are left. | Keeps the original wait. | `cards.c 6 0x6eb2a` |
| Removing a card/city not in the list shifted the list from stack garbage (probably, unreachable). | Keeps the list. | Starts the shift at index 0. | `rules.c @0 0x000000` |
| Removing a calamity not held shifted the list from stack garbage (probably, unreachable). | Keeps the list. | Starts the shift at index 0. | `rules.c @0 0x000000` |

## Calamities

| What was wrong | The port does (default) | `--original` | Where |
|---|---|---|---|
| Discarding a calamity card (Zeus) shifted the card list but not the giver list, so later cards named the wrong giver. | Shifts both lists and resets the freed giver. | Keeps the original mismatch. | `cal1.c 1 0x1e388` |
| Removing a card or city that was not in the list dropped the count and shifted from garbage (probably). | Discards nothing. | Starts at index 0. | `cal1.c 2 0x1e69c` |
| Treachery against a victim with no Treachery card read the giver from garbage (probably, unreachable). | Uses the calamity record's giver. | Uses index 0. | `cal1.c 3 0x20564` |
| "replaced by a African city" / "an Thracian city" (wrong article). | "an" / "a" as the grammar says. | Keeps the original article. | `cal1.c 4 0x2110b` |
| "nullifys", "in %s ." (stray space), "to takeover". | "nullifies", "%s.", "take over". | Keeps the original text. | `cal1.c 5 0xb222c` |
| With an empty choice list the box showed "PIRACY" and wrote into a memory handle (probably). | Shows no name, no highlight, and redraws the map. | Keeps the original list handling. | `cal1.c 6 0x207b4` |
| Epidemic secondary victims: when one nation held all candidate cities, one city was still demanded but the list was empty (probably). | The list is rebuilt correctly; FINISHED works. | Keeps the original empty list. | `cal1.c 7 0x2468c` |
| RESTART in that screen did not restore the victim marks, so a taken city could not be chosen again. | Restores the marks. | Keeps the original restore. | `cal1.c 8 0x2468c` |
| Flood with more than 10 secondary units let the wrong player choose the losses in a mixed game. | The victim chooses its own losses. | Keeps the original choice of player. | `cal2.c 1 0x2d4f0` |
| Epidemic with an empty loss list showed "PIRACY" and corrupted a handle (probably). | No name, no highlight. | Keeps the original list handling. | `cal2.c 2 0x27fcc` |
| After the last listed territory was removed, the buttons stayed enabled on a stale entry and could take a token that was not there (probably). | The buttons test the list count. | Keeps the original tests. | `cal2.c 3 0x27fcc` |
| Volcano/Earthquake: clicking the map could select the volcano (river) whose number equals the clicked territory (probably). | Map clicks are ignored in these lists. | Keeps the original map branch. | `cal3.c 1 0x1eaec` |
| Earthquake pair branch: a map click was matched against a stale list and could select an unrelated pair (probably). | Map clicks are ignored. | Keeps the original branch. | `cal3.c 2 0x1fddc` |
| Civil War "ALL" on tokens did not lower the beneficiary's stock, creating tokens out of nothing. | Lowers the stock as "ONE" does. | Keeps the original counters. | `cal4.c 1 0x2b6ec` |
| "beneficary" (4 messages). | "beneficiary". | Keeps the misspelling. | `cal4.c 2 0xb2fb3` |
| The computer's annexing message always said "Second Faction" even when the first was annexed. | Says "First" or "Second" as annexed. | Keeps the original word. | `cal4.c 3 0xb2ff2` |
| With an empty choice list the Civil War box printed and highlighted an entry although nothing was listed (probably). | The list is bounded. | Keeps the original print. | `cal4.c 4 0x2b6ec` |
| Barbarian losses counted only the first non-zero city loss. | Adds all of them. | Keeps the original count. | `cal5.c 1 0x1dffc` |
| The barbarian director was read from stack garbage when no other nation qualified (probably). | Takes nation 0's path. | Takes nation 0 as the original effectively did. | `cal5.c 2 0x217a0` |
| Runs over equal route/landing entries had no bound and could list territory 0 dozens of times (probably). | The runs are bounded by the data. | Keeps the original runs. | `cal5.c 3 0x000000` |
| Same, for the director's route entries (a.). | Bounded. | Keeps the original run. | `cal5.c 3a 0x1c3d0` |
| Same, for the local landing table (b., stack garbage past the count). | Bounded. | Keeps the original run. | `cal5.c 3b 0x1c9fc` |
| The Epidemic helper passed the city list entry as a territory without subtracting 1, reading the next territory's population limit. | Subtracts 1 like every other reader. | Keeps the original off-by-one. | `helpers2.c @78ecd 0x78ecd` |
| "chooses the %s to flood" misspelling, and the river name "TIGRUS-EUPHRATES". | "chooses", "TIGRIS-EUPHRATES". | Keeps the original text. | `aical2.c 5 0xb35a7` |

## Movement and conflict

| What was wrong | The port does (default) | `--original` | Where |
|---|---|---|---|
| A move log record was written without the play-by-email test (the PBEM block is dropped by ruling). | Writes the record only where the original did with PBEM off. | Keeps the original write. | `move.c @0 0x000000` |
| Same, at the second site. | Same. | Keeps the original write. | `move.c @41805 0x41805` |
| A nation with no movable token got a "-1" highlight that corrupted a memory handle and centred the map on nothing (probably). | No highlight, map centred normally. | Keeps the original -1 handling. | `movephase.c 1 0x55f33` |
| FIND UNMOVED never returned when nothing qualified (a hang in sea mode, probably). | Returns after one full scan. | Keeps the original endless rescan. | `movephase.c 2 0x57810` |
| "ship maint./const.for the other players" (missing space, PBEM only). | "const. for". | Keeps the original text. | `movephase.c 3 0xb5ae4` |
| The city owner was compared with a 0-based row, so hostility was recorded against the wrong nation (probably). | Compares owner with row + 1. | Keeps the original comparison. | `conflict.c 1 0x5c110` |
| The Pirates, Barbarians and factions showed wrong short names ("", "AFRICA", ...) in the conflict screen. | Shows their full names. | Keeps the original table read. | `conflict.c 2 0x5cde8` |
| The Military rule decremented the wrong pair entry (play-by-email only, probably). | Decrements the losing nation's entry. | Reads the zeroed entry past the list (the garbage cannot be reproduced). | `conflict.c 3 0x5b588` |
| MAINTAIN BY LEVY on a second coast left the token marked on the wrong territory (probably). | Uses the canonical territory for both. | Keeps the original territory. | `ship.c A 0x60ddf` |
| The computer's extra ships were sorted by the neighbour territory's tokens (probably). | Sorts by the candidate's own record. | Keeps the original record. | `ship.c B 0x62346` |
| With no coastal candidate, the computer built a ship from stack garbage (probably). | Stops the extra-ship loop. | Keeps the array across nations as the original's frame did, starting at 0. | `ship.c C 0x623ee` |
| Starting a new game leaked three sprite blocks (~2 KB each time, probably harmless). | Frees them. | Keeps the original leak. | `stack.c 1 0x121a0` |
| REPLAY restored the snapshot with record-0 bases, hiding area 160 and nation 11 from the replay view. | Copies the correct ranges. | Keeps the original bases. | `stack.c 2 0xe5d20` |
| A store after freeing the sea-adjacency block wrote into the freed pointer. | Skips the store. | Keeps the original store. | `mapdata.c @ffb5c 0xffb5c` |

## Cities

| What was wrong | The port does (default) | `--original` | Where |
|---|---|---|---|
| The computer's city sites were ordered by the neighbour territory's tokens instead of the site's (probably). | Uses the site's own record. | Keeps the original record. | `city.c 1 0x32214` |
| After a Slave Revolt the REDUCE CITY loop could wait forever on an empty list (probably). | The loop ends. | Keeps the original loop. | `city.c 2 0x5c750` |
| With an empty city list the box showed "PIRACY", read the owner from the wrong byte and corrupted a handle. | Shows no entry while the list is empty. | Keeps the original list handling. | `city.c 3 0x5c880` |

## Advances (ACQUIRE CIVILIZATION)

| What was wrong | The port does (default) | `--original` | Where |
|---|---|---|---|
| Treasury added with the arrow was handed to the next human player when the turn was left with FINISHED (probably). | Clears the used treasury at the start of a human turn. | Keeps the original carry-over. | `adv.c 1 0x162d0` |
| One click on the treasury arrow could turn card credit above the cost into extra "Excess" (probably). | Caps the credit at the cost. | Keeps the original computation. | `adv.c 2 0x18aec` |
| Every ACQUIRE phase re-enabled animations and sound via a one-past-the-array store (the low-memory flag). | Clears only the intended field. | Keeps the original store. | `adv.c 3 0x162d0` |
| A computer's third advance purchase paid for three but counted two, corrupting the message separators (probably). | Counts all purchases. | Keeps the original counter. | `adv.c 4 0x162d0` |

## The computer players

| What was wrong | The port does (default) | `--original` | Where |
|---|---|---|---|
| With three or more advances bought, the message separator after the last name was wrong ("Pottery, \nLaw, \nMusic and \n."). | Prints ", " up to the last but one, " and " before the last, nothing after. | Keeps the original separators. | `aiadv.c 1 0x19700` |
| Item 3 of the stale words above (a read of an unwritten stack word). | The word starts at 0 (both modes: the garbage cannot be reproduced). | Same as default. | `aiadv.c 2 0x000000` |
| A stack word read before it was written in the desirability routine (probably unreachable). | Starts at 0 (both modes). | Same as default. | `aiadv.c 1 0x1a888` |
| Out-words of the card-paying routine could be stale on a later call. | Keeps them as locals across calls, as the original. | Same as default. | `aiadv.c 2 0x1b59c` |
| A stack word read before written when paying from treasury alone (probably). | Starts at 0 (both modes). | Same as default. | `aiadv.c 3 0x1be08` |
| Piracy could give the Pirates a 10th city, written one past their city list (probably). | Respects the computed room. | Keeps the original count. | `aical1.c 1 0x24eb8` |
| With no token candidate, the computer converted the territory of an uninitialised stack record (probably). | Converts nothing (both modes for the garbage). | Starts at index 0. | `aical1.c 2 0x21e8c` |
| Famine planning could plan more tokens for a territory than it holds (probably). | Plans only what the territory holds. | Keeps the original planning. | `aical2.c 1 0x260c4` |
| Annexing a faction did not update its board/stock counters, so later tests disagreed with the board. | Updates both counters. | Keeps the original counters. | `aical2.c 2 0x2c894` |
| The annexing pass could loop forever when it moved nothing (probably a hang). | A pass that moves nothing ends the loop. | Keeps the original loop, including the hang. | `aical2.c 3 0x2d1c0` |
| With an empty list the annexing box showed "PIRACY" and corrupted a handle (probably, unreachable). | No name, no highlight. | Keeps the original list handling. | `aical2.c 4 0x2c894` |
| The computer's discard loop could wait forever with no commodity left (probably a hang). | The loop ends. | Keeps the original loop. | `aicards.c 1 0x6eda8` |
| A counter used before it was set on early exits in city planning. | Starts at 0 (both modes: the garbage cannot be reproduced). | Same as default. | `aicity.c 1 0x33964` |
| Several locals were left uninitialised but filled by callees before any read. | Zeroed (both modes); no behaviour depends on it. | Same as default. | `aicity.c 2 0x3580c` |
| The route-choice routine could pick one past the row on a second pick (probably). | Clamps the pick. | Keeps the original pick. | `aiq.c 1 0x7701c` |
| Ships on a second coast read the land matrix past its rows (in the original: heap address bytes). | Uses the alias row. | Keeps the original out-of-range read. | `aisea.c 1 0xff648` |
| A test that was always written before read (no garbage actually read). | Kept as checked. | Keeps the original. | `aisea.c 2 0x4b567` |
| The hop after stop 5 read one byte past a 6-byte leg (stack padding, both modes read 0xFF). | Reads the leg's end. | Same as default. | `aisea.c 3 0x000000` |
| Replay could read past the plan records (unreachable). | The records are zeroed and the scan bounded. | Keeps the original (port guard in both modes). | `aisea.c 4 0x000000` |
| A scan for tied routes had no bound (stopped by data). | Bounded by the data object. | Keeps the original. | `aisea.c 5 0x000000` |
| The unload sweep could repeat forever in theory (unreachable). | A sweep that unloads nothing ends it. | Loops as the original. | `aisea.c 6 0x000000` |
| best[] had 256 entries but ship tables 160 (unreachable). | Bounded by the table. | Keeps the original (port guard). | `aisea.c 7 0x000000` |
| With no route the tie count was 0 and 30000 / 0 trapped (a crash, unreachable). | Returns -1 (both modes). | Same as default. | `aisea.c 8 0x65bcb` |
| A scrap array was read past the written entries when a nation had fewer than 4 ships (probably). | Reads 0 past the written entries (both modes). | Same as default. | `aiship.c 1 0x61859` |
| Two type loops could spin forever in theory (unreachable). | The loop stops. | Keeps the original (port guard, no change in play). | `aiship.c 2 0x63e33` |
| A round robin could spin with excess left (unreachable). | A pass that takes nothing stops. | Keeps the original (port guard). | `aiship.c 3 0x68637` |
| A stack copy of the trade rows could overwrite the pair list and the return address with 56+ rows (probably). | Models the frame up to 90 rows. | Keeps the original model where byte-exact. | `aitrade.c 4 0x6a537` |
| A card read from stack garbage when no candidate was ever tried. | Starts at 0 (both modes). | Same as default. | `aitrade.c 1 0x697ea` |
| A counter read even if its routine never ran (no effect). | Starts at 0 (both modes). | Same as default. | `aitrade.c 2 0x6a3c1` |
| Trade tables read rows of nations not in play and past the card row (stack garbage). | Unwritten rows read 0 (both modes). | Same as default. | `aitrade.c 3 0x1104c8` |

## Screens and text

| What was wrong | The port does (default) | `--original` | Where |
|---|---|---|---|
| A shared text routine stepped past the first byte of a line that starts the text. | Steps correctly. | Keeps the original step. | `ui_text.c @3a727 0x3a727` |
| The City Support phases showed the Ship Construction help; CITYSUP.HLP was never shown. | Shows the City Support help. | Keeps the original help file. | `menu1.c @0 0x000000` |
| A column store was not clamped to the 45 screen columns. | Clamps it. | Keeps the original store. | `menu2.c @0 0x000000` |
| A colour store could keep the last round label's colour (7) when it was skipped. | Sets the colour explicitly. | Keeps the original store. | `menu2.c @0 0x000000` |
| A nation button in these loops was compared as P - 1 instead of 1-based. | Compares 1-based. | Keeps the original comparison. | `panel.c @0 0x000000` |
| A row button's bottom 4 pixels selected the next nation's row. | The rows are exactly 44 high. | Keeps the original hit test. | `info.c @0 0x000000` |
| A saved junk screen kept row 479 from the previous screen. | Saves all 480 rows. | Keeps the original 479-row save. | `info.c @0 0x000000` |
| After a conquest, the shared CONTINUE button stayed labelled "EXIT" in later dialogs (probably). | Restores the label. | Keeps the original relabel. | `end.c 1 0x14294` |
| "popped once to often" (three copies). | "too often". | Keeps the original text. | `mstack.c @b96a5 0xb96a5` |
| "Montheism" (two strings). | "Monotheism". | Keeps the original text. | `mono.c @b1eea 0xb1eea` |
| "World State Stack has popped once to often." | "...too often.". | Keeps the original text. | `stack.c @b9679 0xb9679` |

Generated from the notes on 2026-11-21 by `tools/verify/fixes.py list`; the sentences are Mistral's.
