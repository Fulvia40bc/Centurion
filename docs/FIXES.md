# Fixes for the 1995 original's bugs

The 1995 DOS game (Advanced Civilization) contains bugs. This port fixes them **by default**. Running with
`--original` keeps the original behaviour wherever it can be reproduced. A few fixes apply **in both modes**: where the
original read memory it never wrote (which cannot be reproduced) or could crash or overwrite memory, the port does
the same sane thing either way. Spelling and wording slips in the messages are fixed the same way, per Erik's ruling of
19 November (`docs/TEXT_SWEEP.md`); `--original` keeps the original text.

The table has 112 rows: **85** fixes active by default (`--original` brings the original behaviour back),
**16** fixed in both modes, and **11** text fixes. They come from the notes in the port's source
(`tools/verify/fixes.py list`).

"(probably)" in a row means the effect on the game is inferred rather than seen in the code; the reading of the
code itself is solid. "(probably never happens)" means the notes judge that ordinary play cannot reach the bug,
also by inference.

## Setup (nation choice, taxes and population at round start, saving and loading)

| What was wrong | The port does (default) | `--original` | Where |
|---|---|---|---|
| When the computer picked its nations, nine places added to the count of nations still to give instead of taking one off, so for some human choices the computer got one or two nations more than the setup asked for. | Takes one off everywhere. | Keeps the original count. | `nations.c A 0x30339` |
| With 7 players, when the humans held Asia, Assyria and Babylon and one of Illyria, Thrace or Crete, the computer still took a nation when none was left to give: one too many. | Takes it only if one is left to give (the random draws stay the same). | Keeps the extra nation. | `nations.c B 0x30aae` |
| A 4-player game of Iberia, Illyria, Thrace and Crete (no Africa, only possible with 4 humans) gave the human Illyria the wrong starting map (the list of which starting territories are good), because a check was copied from Africa's without changing the nation. | Checks for Africa, as the map's label intends. | Keeps the copied check. | `aistart.c @15254 0x15254` |
| Restoring a save the game then refused (damaged, or an older format) had already replaced the game in progress with that file before the refusal was shown. | Checks the file first and loads it only if it passes. | Loads first, then refuses (the game is already replaced). | `save.c 1 0x5ff28` |
| On the "Restoring" screen the save's name was measured in a smaller font than the one it is printed in, so a long name ran off both sides of the screen. | Measures in the font it prints with (and switches to the smaller font for a long name). | Keeps the original measuring. | `save.c 2 0x5ff28` |
| While typing a save's name, the CANCEL button could not be clicked (only Esc worked): the game never looked at the mouse there. | Looks at the mouse after each key, so CANCEL works. | CANCEL cannot be clicked (Esc works). | `save.c 3 0x60694` |
| Leaving the save-name box with Esc left the drawing aimed at a hidden off-screen page, so the next screen could be drawn where it is not seen (probably). | Aims the drawing back at the screen before leaving. | Keeps the original. | `save.c 4 0x60694` |
| Very old save files (from before the game added its name tag to the file) were read from the wrong place after the header, so everything in them came out shifted (probably; such files are refused anyway). | Skips the tag only when the file has one. | Keeps the shifted reading. | `saveload.c A 0x5f0e0` |
| Two old save formats kept the previous game's table of random numbers instead of making a new one (probably; such files are refused anyway). | Makes a new table. | Keeps the old table. | `saveload.c B 0x5f7e5` |
| "Democrary spares %s from a Tax Revolt." | "Democracy spares %s ...". | Keeps the misspelling. | `tax.c 1 0xb8904` |
| During a tax revolt the player buttons highlighted nation 9 (or nobody) instead of the revolting nation. | Highlights the revolting nation. | Keeps the wrong highlight. | `tax.c 2 0x66408` |
| In a tax revolt, when the player had to give up every one of its cities, the empty choice box showed the name "PIRACY", and its highlight wrote into the game's link to the sea-connection data, damaging it. | Shows no name and no highlight while the box is empty. | Keeps the name and the damaging write. | `tax.c 3 0x66ff8` |
| In manual population expansion, DISTRIBUTE ALL could go round forever when tokens were left but no listed territory could take one (probably never happens with a sound map). | Stops when a round places nothing. | Keeps the endless loop. | `pop.c 1 0x67d1a` |

## Trade (offers, counter-offers, buying goods)

| What was wrong | The port does (default) | `--original` | Where |
|---|---|---|---|
| A complete offer with a bluff but no replacement card showed the warning "You must declare a bluff replacement." after leftover junk text. | Shows the warning alone (in both modes: the junk cannot be reproduced). | Same as default; the 1995 game showed the junk. | `trade.c 1 0x71768` |
| An accepted counter-offer saved a junk number as the number of cards the player wants (seen in a 1995 replay: 7 where it should be 1); the computers read this number (probably). | Keeps the number from the player's own offer. | Stores 0 (the junk cannot be reproduced). | `trade.c 2 0x71768` |
| Clicking the two lowest pixel rows of the card chart could pick a calamity card as a bluff or wanted card (probably). | Ignores clicks outside the chart. | Keeps the original hit test. | `trade.c 3 0x7109c` |
| If the trade screen removed a calamity the player's record did not hold, the card list was shifted from a junk position and lost a card (probably never happens: the cards come from the same hand). | Leaves the record as it is. | Shifts from the start of the list (the first card is lost). | `trade.c 4 0x70984` |
| If an offering nation had no public trade record, the trade log took its cards from a junk record (probably never happens: every nation with an offer has a record). | Logs no cards for that nation. | Takes the cards of the first record. | `trade.c 5 0x75074` |
| "The responses to African offer..." missed the article. | "The responses to the African offer...". | Keeps the original wording. | `trade.c 6 0xb92a3` |
| Clicking between the card stacks (or right of the last) bought from a stack that does not exist: 58 gold paid for whatever card that bit of memory held. | Ignores the click. | Keeps the original purchase. | `cards.c 1 0x6d8e8` |
| The first stack the player could not afford (drawn empty) could still be clicked and bought, driving the treasury below 0. | Only stacks the treasury can pay for can be clicked. | Keeps the original. | `cards.c 2 0x6d8a7` |
| CANCEL after starting a purchase left the purchase counted (probably only visible in saves). | CANCEL takes the count back. | Keeps the counted purchase. | `cards.c 3 0x6d800` |
| Without the variant rule, BUY took the top card of an empty stack unchecked: paid 18 gold, got nothing (probably). | Buys, pays and counts nothing. | Keeps the original behaviour. | `cards.c 4 0x6d5f3` |
| The computer's "Buy" counter showed 0 when it had bought cards but its last try did not buy (probably only the screen). | Counts the purchases. | Keeps the original counter. | `cards.c 5 0x6c6c1` |
| With more than 8 cards and no commodity left, the discard screen waited forever (probably). | The loop ends when only calamities are left. | Keeps the original wait. | `cards.c 6 0x6eb2a` |
| Removing a city that was not in the nation's city list shifted the list from a junk position: a real city was lost (probably; not yet checked whether this can happen). | Leaves the list as it is. | Shifts from the start of the list (the first city is lost). | `rules.c 1 0x78f04` |
| Removing a calamity card the nation did not hold shifted its card list from a junk position: a real card was lost (probably; not yet checked whether this can happen). | Leaves the list as it is. | Shifts from the start of the list (the first card is lost). | `rules.c 2 0x75ca8` |

## Calamities

| What was wrong | The port does (default) | `--original` | Where |
|---|---|---|---|
| Discarding a calamity card (Zeus) shifted the card list but not the giver list, so later cards named the wrong giver. | Shifts both lists and resets the freed giver. | Keeps the original mismatch. | `cal1.c 1 0x1e388` |
| Removing a calamity card or a city that was not in its list still lowered the count and moved the list using a leftover memory value (probably, unreachable). | Leaves the hand or city list as it is. | Starts the move at the first entry. | `cal1.c 2 0x1e69c` |
| Treachery against a victim with no Treachery card read the giver from garbage (probably, unreachable). | Uses the calamity record's giver. | Uses index 0. | `cal1.c 3 0x20564` |
| "replaced by a African city" / "an Thracian city" (wrong article). | "an" / "a" as the grammar says. | Keeps the original article. | `cal1.c 4 0x2110b` |
| "nullifys", "in %s ." (stray space), "to takeover". | "nullifies", "%s.", "take over". | Keeps the original text. | `cal1.c 5 0xb222c` |
| In the city choosers of Treachery, Flood, Barbarians and Piracy, an empty list showed the name "PIRACY", damaged a memory pointer and centred the map on a territory that does not exist (probably). | Shows no name and no highlight, and leaves the map as it is. | Keeps the original behaviour. | `cal1.c 6 0x207b4` |
| Piracy, the second victims: when one nation held all the coastal cities in question (e.g. any 3-player game with a traded Piracy), the list emptied while a city was still owed. FINISHED stayed off, and REMOVE handed a city that does not exist to the Pirates (probably). | When the list is empty nothing more is owed, so FINISHED works. | Keeps the original: FINISHED stays off. | `cal1.c 7 0x2468c` |
| Piracy, the second victims: RESTART put the cities back but not the nations' marks, so a nation that had already given a city could not be chosen again. | Restores the marks too. | Keeps the marks unrestored. | `cal1.c 8 0x2468c` |
| Flood with more than 10 secondary units: whether each neighbour's losses were picked on screen or by the computer depended on the victim, not on that neighbour. In a mixed game a human victim picked a computer's losses, and the computer picked a human neighbour's. | Each nation's losses are picked its own way (a human on screen, a computer by its routine). | Keeps the victim's way for everyone. | `cal2.c 1 0x2d4f0` |
| The unit-loss screen with an empty list (e.g. an Epidemic victim with no units) showed the name "PIRACY" and damaged a memory pointer (probably). | Shows no name and no highlight. | Keeps the original behaviour. | `cal2.c 2 0x27fcc` |
| On the unit-loss screen, after the last listed territory was taken out, REMOVE ONE, REMOVE ALL and REDUCE CITY stayed on and acted on that old territory, and could take a token that was not there (probably). | Those three buttons are off while the list is empty (RESTART stays). | Keeps the buttons on. | `cal2.c 3 0x27fcc` |
| Choosing a volcano (Volcano) or a river (Flood): a click on the map picked the volcano or river by number rather than by place, so clicking SUDETEN, SCYTHIA or KUBAN picked one, and clicking the volcano's own territory picked nothing (probably). | Map clicks are ignored there; the arrows choose. | Keeps the map clicks. | `cal3.c 1 0x1eaec` |
| Earthquake, choosing the pair of cities: a map click was checked against a list left over from an earlier screen and could pick an unrelated pair, or none (probably). | Map clicks are ignored; the arrows choose. | Keeps the map clicks. | `cal3.c 2 0x1fddc` |
| Civil War "ALL" on tokens did not lower the beneficiary's stock, creating tokens out of nothing. | Lowers the stock as "ONE" does. | Keeps the original counters. | `cal4.c 1 0x2b6ec` |
| "beneficary" (4 messages). | "beneficiary". | Keeps the misspelling. | `cal4.c 2 0xb2fb3` |
| The computer's annexing message always said "Second Faction" even when the first was annexed. | Says "First" or "Second" as annexed. | Keeps the original word. | `cal4.c 3 0xb2ff2` |
| Civil War: when the unit list ran empty (taking a city with fewer than 5 points left to take), the box still showed and highlighted the territory just taken (probably). | Shows no name and no highlight while the list is empty. | Keeps the old name and highlight. | `cal4.c 4 0x2b6ec` |
| Barbarians: the losses sum added only the first nation's city losses. No effect on play (the sum is only tested for zero); only some internal search bytes differ. | Adds all of them. | Keeps the original sum. | `cal5.c 1 0x1dffc` |
| Barbarians: when no other nation scored above 0, the nation steering the barbarians was taken from a leftover memory value (probably). | Takes the best-scoring other nation in play, even with a score of 0 or less (the victim only if nobody else plays). | Takes nation 0, which sends the barbarians the computer's random way (the original's leftover value cannot be reproduced). | `cal5.c 2 0x217a0` |
| Barbarian moves: when all 10 route entries were equal, the list ran on past its end and named SUDETEN up to 39 more times (probably; only possible from Syria). | Stops after the 10th entry. | Keeps the long run. | `cal5.c 3a 0x1c3d0` |
| Barbarian landing list: the list ran past its end. Iberia with 7 tokens, or Egypt with 12, in each of its six landing zones got an extra SUDETEN entry, and past the end the original read leftover memory (probably). | Stops at the end of the list. | Keeps the extra entry where it can be reproduced; stops where the original read leftover memory. | `cal5.c 3b 0x1c9fc` |
| Epidemic, for a nation with no spare tokens: each city's value was worked out from the population limit of the next territory instead of its own. | Uses the city's own territory. | Keeps the next territory's limit. | `helpers2.c 1 0x2ec94` |
| "%s choses the %s to flood." (the computer's Flood message). | "chooses". | Keeps the misspelling. | `aical2.c 5 0xb35a7` |
| The river name "TIGRUS-EUPHRATES" in the two Flood messages and the river chooser. | "TIGRIS-EUPHRATES". | Keeps the misspelling. | `cal2.c @b36d4 0xb36d4` |

## Movement and conflict

| What was wrong | The port does (default) | `--original` | Where |
|---|---|---|---|
| Pressing NEXT SHIP wrote a play-by-email log entry even in an ordinary game; with no log there, each press wrote two bytes into the PC's own system memory, a little further each time (probably). | Writes the entry only in play-by-email games, as the other ship buttons do (so never in the port). | Counts the entry as the original did; the stray write itself is skipped (it cannot safely be reproduced). | `move.c 1 0x41730 0x41805` |
| A human nation with nothing left to move on the board (only cities or ships) had the map jump to an empty spot labelled "PIRACY", and a stray write spoiled a memory pointer, which could crash a later START NEW GAME (probably). | No highlight; the map shows the nation's home area. | Keeps the original jump, label and stray write. | `movephase.c 1 0x55f33` |
| FIND UNMOVED never returned when nothing qualified (a hang in sea mode, probably). | Returns after one full scan. | Keeps the original endless rescan. | `movephase.c 2 0x57810` |
| "ship maint./const.for the other players" (missing space, PBEM only). | "const. for". | Keeps the original text. | `movephase.c 3 0xb5ae4` |
| When a city with no defending tokens was attacked, the hostility was noted for the wrong nation: the city's owner got none towards the attackers, and the next nation in the list got it instead (probably). | Notes it for the city's owner. | Keeps the original mix-up. | `conflict.c 1 0x5c110` |
| In the losses box after a conflict, the Pirates showed no name, the Barbarians "AFRICA" and the two Civil War factions "IBERIA" / "ILLYRIA" (when their full name was too wide for the column). | Shows their full names. | Keeps the wrong names. | `conflict.c 2 0x5cde8` |
| In play-by-email games, the rule that Military nations cost the others a token checked and lowered the wrong number (unset memory just past the list) instead of the losing nation's count (probably). | Takes the token from the nation that loses it (never runs in the port: play-by-email is left out). | Reads an empty entry there instead (the original's leftover memory cannot be copied), so the rule takes no token. | `conflict.c 3 0x5b588` |
| "%s,you have no stock to pillage with." (missing space). | "%s, you have no stock to pillage with." | Keeps the original text. | `conflict.c @b5cde 0xb5cde` |
| The conflict phase had room for 60 contested territories; a 61st would overwrite other data (probably never happens in play). | Room for all 157 (both modes: the overwrite cannot be reproduced). | Same as default. | `conflict.c 4 0x5aaa4` |
| A city attack where only Civil War faction tokens stood used an attacker number that was never set (probably never happens in play). | No attack (both modes: the unset number cannot be reproduced). | Same as default. | `conflict.c 5 0x5b224` |
| The losses box divided by the number of nations with losses; with none it would crash (probably never happens in a conflict). | Draws nothing then (both modes). | Same as default. | `conflict.c 6 0x5ce6a` |
| When a ship on a second coast was kept by paying the nation's last token there (MAINTAIN BY LEVY), the territory still counted as the nation's, with 0 tokens (probably). | Clears the nation's mark on that territory, as Ship Construction does. | Keeps the stale mark. | `ship.c A 0x60ddf` |
| The computer ranked the coasts for its extra ships by the tokens in the territory numbered one below each coast, not the coast's own (probably; probably never happens either, as a setting the extra ships need is never switched on). | Ranks by the coast's own tokens. | Keeps the original ranking. | `ship.c B 0x62346` |
| With no coast to build on, the computer still built an extra ship, at a site taken from leftover memory (probably; probably never happens either, as a setting the extra ships need is never switched on). | Builds no extra ship. | Builds at the first site found for an earlier nation, or at territory 0 for the first (the original's leftover memory cannot be copied). | `ship.c C 0x623ee` |
| Starting a new game never freed 15 small token pictures, so about 2 KB of memory was lost each time (probably harmless). | Frees them. | Keeps the original leak. | `stack.c 1 0x121a0` |
| REPLAY showed the board one record off: the last territory record and the Barbarians were not shown as after movement, and two spare records kept the replay's copy afterwards (probably nothing visible). | Copies exactly the records the game saves and restores around the replay. | Keeps the shifted copy. | `stack.c 2 0x4059a` |
| When a game was cleared (START NEW GAME), the link to the freed route data (N.RL) was left set, because the clearing went to the sea-adjacency link instead (probably harmless: the next game loads the route data again). | Clears the route-data link. | Keeps the stale link. | `mapdata.c 1 0x3f5c7` |

## Cities

| What was wrong | The port does (default) | `--original` | Where |
|---|---|---|---|
| When a computer with Architecture had more possible city sites than free cities, it ranked them by the tokens in the territory numbered one below each site, not the site's own (probably). | Ranks by the site's own tokens. | Keeps the original ranking. | `city.c 1 0x32214` |
| After a Slave Revolt, a nation that had lost all its cities could be stuck for ever in the city-reduction step: a human on an empty REDUCE CITY screen, a computer in an endless loop (probably). | With no city left the step ends. | Keeps the endless wait. | `city.c 2 0x5c750` |
| With no city to choose, the city-reduction box showed the name "PIRACY", and the highlight and REDUCE CITY wrote into memory they should not touch (probably). | The box stays empty and REDUCE CITY just closes it. | Keeps the original list handling. | `city.c 3 0x5c880` |
| With Monotheism, a city that may not be converted (its owner has Monotheism or Theology, it is the Pirates' city, or the converter already has 9 cities) could still be offered through other nations' tokens there and converted, which could give a nation a 10th city (probably never happens in play). | Such a territory is not offered. | Keeps the original list. | `mono.c 1 0x227b0` |

## Advances (ACQUIRE CIVILIZATION)

| What was wrong | The port does (default) | `--original` | Where |
|---|---|---|---|
| Treasury a human set aside with the arrow and then left unused (FINISHED) carried over to the next human: it was taken from that player's treasury, which could drop below 0 (probably). | Each human's turn starts with no treasury set aside. | Keeps the carry-over. | `adv.c 1 0x162d0` |
| When an advance's card credit was above its price (Pottery and Clothmaking cost 45, credit can reach 60), one click on the treasury arrow added the surplus credit to "Excess", which then counted as free points toward the next purchase (probably). | Counts the credit only up to the price. | Keeps the original sum. | `adv.c 2 0x18aec` |
| On a PC with too little memory, sound and animations are locked off at start; every ACQUIRE CIVILIZATION phase removed that lock, so the AUDIO/VISUALS menu could turn them back on. | Leaves the lock in place. | Removes the lock as the original did. | `adv.c 3 0x17674` |
| A computer buying a third advance in one turn paid for three but got only two (probably). | Counts the third, so it gets all three it paid for. | Keeps the original count. | `adv.c 4 0x17008` |

## The computer players

| What was wrong | The port does (default) | `--original` | Where |
|---|---|---|---|
| When a computer bought three or more advances at once, the message put the "and" after the last name: "Pottery, Law, Music and ." | "Pottery, Law and Music." | Keeps the original wording (never seen there: with `--original` a computer buys at most two at once). | `aiadv.c 1 0x19700` |
| When a computer paid for advances from its treasury alone (holding no goods cards), it read a leftover value from memory while working out the payment (probably). | Uses 0 there (both modes: the leftover value cannot be reproduced). | Same as default. | `aiadv.c 3 0x1be08` |
| Piracy could give the Pirates a 10th city (the rule allows 9), which spilled into the next part of the game's records and spoiled later Piracy (probably). | Gives the Pirates no more cities than they have room for. | Keeps the original (a 10th city can happen). | `aical1.c 1 0x24eb8` |
| When a computer converting by Monotheism found no territory it could take, it converted a territory picked from leftover memory; a wild value could crash the game (probably). | Converts nothing and shows no message. | Converts territory 0 (a stand-in: the original's leftover value cannot be reproduced). | `aical1.c 2 0x21e8c` |
| When a calamity such as Famine or Epidemic struck a computer, it could plan to remove more tokens from a territory than were there (the count went below zero), and with nothing left to remove it planned forever (a hang) (probably). | Never removes more than a territory holds; a round that removes nothing ends the planning. | Keeps the original planning, including the hang. | `aical2.c 1 0x260c4` |
| In a Civil War, when a computer took tokens away from the new faction, the faction's counts of tokens on the board and in stock were not updated, so later checks disagreed with the map. | Updates both counts. | Keeps the original counts. | `aical2.c 2 0x2c894` |
| In a Civil War, a computer taking over the faction's tokens could loop forever when there was nothing left to move (a hang, caused by the wrong counts above) (probably). | A pass that moves nothing ends the loop. | Keeps the original loop, including the hang. | `aical2.c 3 0x2d1c0` |
| In the same Civil War step, with an empty list the box showed the name "PIRACY" and damaged a memory pointer (probably never happens). | Shows no name and no highlight. | Keeps the original. | `aical2.c 4 0x2c894` |
| When a computer had to discard down to 8 cards but held no goods cards to discard, it could loop forever (a hang) or overwrite memory (probably). | The discarding stops. | Keeps the original loop, including the hang. | `aicards.c 1 0x6eda8` |
| When a computer planning new city sites had none left to plan, it added a counter it had never set (a leftover value from memory) (probably). | The counter starts at 0 (both modes: the leftover value cannot be reproduced). | Same as default. | `aicity.c 1 0x33964` |
| When a computer shuffled its list of possible city sites, the shuffle could take one entry past the end of the list: a leftover value from memory went into the list and a real site was lost (probably; about 1 in 6,000). | Takes the last entry instead. | Takes the entry past the end, which reads 0 here (the original's leftover value cannot be reproduced). | `aiq.c 1 0x7701c` |
| A computer ship in a territory with a second coast (territories 157 to 159) looked up its land neighbours in a table row that does not exist, reading memory addresses as if they were the map (probably). | Uses the row of the territory that coast belongs to. | Reads the same bytes as the 1995 game (rebuilt from its memory layout). | `aisea.c 1 0x64bb8` |
| A computer ship with 5 movement points could read one byte past the end of one leg of its route, unused memory in the original (probably). | Reads it as "end of leg" (both modes: the original's value cannot be reproduced). | Same as default. | `aisea.c 3 0x4ac50` |
| When a computer replayed a planned ship move, it could read past the end of the plan (probably never happens). | The plan is cleared first and the search stops at its end (both modes). | Same as default. | `aisea.c 4 0x36918` |
| A search for sea routes as good as the best one had no limit of its own; it stopped only because of the data that follows. | Also stops at the end of the game's data (both modes; no change in play). | Same as default. | `aisea.c 5 0x36918` |
| Unloading a computer's ship could repeat forever (a hang) (probably never happens). | A round that unloads nothing ends it. | Keeps the original loop, including the hang. | `aisea.c 6 0x65010` |
| Two ship tables could in principle be read or written past their end (never happens: a nation has at most 4 ships). | Checks the limits (both modes; no change in play). | Same as default. | `aisea.c 7 0x4d29c` |
| With no sea route at all, a division by zero would crash the game (probably never happens). | Chooses no route instead (both modes). | Same as default. | `aisea.c 8 0x65bcb` |
| When a computer with fewer than 4 ships decided which ships to keep, it read leftover memory for the ships it did not have (probably). | Treats the missing ships as 0 (both modes: the leftover values cannot be reproduced). | Same as default. | `aiship.c 1 0x61859` |
| Two loops in a computer's choice of ships to build could run forever (never happens). | The loop stops (both modes; no change in play). | Same as default. | `aiship.c 2 0x63e33` |
| When a computer short of stock placed its new tokens, a sharing-out loop could run forever with nothing left to share (never happens). | A pass that shares out nothing stops (both modes; no change in play). | Same as default. | `aiship.c 3 0x68637` |
| When a computer answered trade offers and the list held more than 55 entries, its bookkeeping overwrote its own list of offer pairs, and above 90 it could crash the game (probably). | Uses a list big enough for every entry. | Repeats the overwriting up to 90 entries; above 90 (a crash in 1995) it uses the big list too. | `aitrade.c 4 0x6a537` |
| When a computer chose which card to name in a trade offer and had tried none, it could use a card number from leftover memory (probably never happens). | Uses 0 (both modes: the leftover value cannot be reproduced). | Same as default. | `aitrade.c 1 0x697ea` |
| The computer's trade tables could read rows that were never filled in (nations not in play, or a card number above 18), i.e. leftover memory (probably never happens by default). | Those rows read 0 (both modes). | Same as default. | `aitrade.c 3 0x6ad70` |

## Screens and text

| What was wrong | The port does (default) | `--original` | Where |
|---|---|---|---|
| When a word was too wide for its text box, the box was widened by one pixel but the word's first letter was dropped each time, so long words lost letters. | Widens the box until the whole word fits. | Drops the letters as the original did. | `ui_text.c @3a727 0x3a727` |
| In the City Support phases, HELP showed the Ship Construction help; the City Support help in the game's files was never shown. | Shows the City Support help (titled "City Support"). | Shows the Ship Construction help. | `menu1.c 1 0x42cec` |
| From round 45 on, the Scoring History graph wrote each score into the next nation's row (Egypt's went past the table, onto a memory pointer the game later frees). | Keeps the score in the graph's last column. | Keeps the original. | `menu2.c 1 0x44df2` |
| In the Scoring History graph, the word "R O U N D S" came out grey instead of white in games of 37 to 44 rounds with an odd number of rounds. | Always white. | Keeps the original colour. | `menu2.c 2 0x4520b` |
| On the whole-board overview (ZOOM), where ships of several nations share a territory, the ships of the nation numbered one below the viewer's were drawn on top instead of the viewer's own. | Draws the viewer's own ships on top. | Keeps the original order. | `panel.c 1 0x3e374` |
| In Civilization Tools, a click on the bottom edge of a row (or a click dragged off it) showed the wrong advance, and in the last column could crash the game or show garbage (probably). | Keeps the click inside the row clicked. | Keeps the original. | `info.c 1 0x1a0b9` |
| After REVIEW (Trading Wrapup) in the trade screens, the bottom line of the screen kept the wrap-up picture's line instead of the trade screen's (probably: it shows only where the two differ). | Puts back the whole screen. | Keeps the original (one line short). | `info.c 2 0x3fc75` |
| After a game ended by conquest, a new game started in the same session showed "EXIT" on the CONTINUE button of messages, elimination screens and the AST chart. | Puts the CONTINUE label back. | Keeps "EXIT". | `end.c 1 0x14294` |
| The error message shown when the game stops on an internal fault said "popped once to often" (three copies). | "popped once too often". | Keeps the original text. | `mstack.c @b96a5 0xb96a5` |
| "Montheism" (two strings). | "Monotheism". | Keeps the original text. | `mono.c @b1eea 0xb1eea` |
| A fourth copy of the same internal-fault message, "World State Stack has popped once to often." | "...too often." | Keeps the original text. | `stack.c @b9679 0xb9679` |
| Five obvious slips in the help texts: "Acquistion", a stray full stop in 'Salt?".', "button , only", "a F (" and "the affect". | Corrected when the help is shown. | Keeps the original text. | `menu1.c @4290c 0x4290c` |

Generated from the notes on 2026-11-21 by `tools/verify/fixes.py list` (the sentences are Mistral's); checked
row by row against the code and corrected on 2026-11-22 by Claude.
