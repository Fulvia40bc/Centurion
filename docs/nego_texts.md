# Negotiation texts (MNEGO TEXTA..TEXTZ.LZW), all 26 provinces

Generated from the game data (MNEGO archive 7, `C:\work\Centurion-native\MNEGO.DAT`) and the static DS (OVL0.OVL at 0xB400) by
the decoder in NEGO_DATA_NOTES.md. Claims: the texts and tables are **D** (decoded, dumped byte-exact); the state numbering and
the "->" transitions follow the 5F82 dialogue loop (LEGIONS_NOTES 5F82 + NEGO_DATA_NOTES section 3) and are **D**; nothing here
was run in the game (no **V**).

How to read a section:
- Letter = `'@' + z` (0x6570 writes `0x40 + z` into "textA.lzw" / "leaderA.cbm"), so zone 1 = A ... zone 26 = Z. Zone 0 (Italia) has none.
- **Leader** = DS:5F1C[z] (used by the scout report 0x7B56: "<leader>, the leader of <zone>, offers a greeting:").
- **Scout** = SCOUT.LZW string z (said first, colour 0, by 0x7B56 when the screen opens).
- **Gate**: the talk starts in state 1 (friendly) when rank [5971] >= 4092[z] and [634E] >= 40AE[z], else in state 0 (hostile).
  **Tribute**: 40CA[z]; an offer Low/Medium/High (0/1/2) is accepted (state 2) when 40CA[z] > level, else state 3.
- Each **state s** = string s of TEXTx: what the province says (colour 15). `~` separates variants; 0x6501 rolls roll(3) and takes
  variant roll+1, the last one if there are fewer (so with 2 variants the 2nd is shown 2 times in 3).
- Under it, the reply menu (2 or 3 items, from table 42EC[z]): **Reply label** [hotkey] "the player's line (colour 9)" and the next
  state. Where the next state depends on the mood (mood = [634E] / 34, minus 1 if the province's +0x0E > 75 and mood > 0) or on
  "weak" (legion infantry + cavalry < the province's), it is listed as `mood m: s / weak s'`.
- Terminal: Withdraw / Go to War end the talk; state 2 = alliance (ends, except in Aegyptus). Strings that no path reaches are marked.

| z | letter | zone | leader (5F1C) | min rank 4092 | min [634E] 40AE | tribute 40CA | B table 42EC | A table 455A (c) |
|---|---|---|---|---|---|---|---|---|
| 1 | A | Carthago | Hannibal | 4 | 70 | 1 | 4226 | 442E (12) |
| 2 | B | Aegyptus | Cleopatra | 6 | 80 | 2 | 4162 | 433E (19) |
| 3 | C | Macedonia | Philip of Macedon | 4 | 60 | 2 | 42A0 | 44E2 (11) |
| 4 | D | Gaul | Vercingetorix | 2 | 40 | 1 | 41A2 | 43B0 (10) |
| 5 | E | Parthia | Darius the Great | 4 | 70 | 1 | 4226 | 442E (12) |
| 6 | F | Syria | Antiochus | 4 | 50 | 1 | 424E | 4476 (10) |
| 7 | G | Thracia | Thorax of Thrace | 2 | 60 | 2 | 42A0 | 44E2 (11) |
| 8 | H | Pontus | Mithridates | 3 | 40 | 2 | 424E | 4476 (10) |
| 9 | I | Asia | Attalus | 2 | 30 | 3 | 424E | 4476 (10) |
| 10 | J | Cilicia | Absolna the Rotten | 4 | 10 | 1 | 4278 | 44B2 (8) |
| 11 | K | Alpes | Vindelic | 0 | 10 | 1 | 41A2 | 43B0 (10) |
| 12 | L | Germania | Arminius | 2 | 30 | 1 | 41D0 | 43B0 (10) |
| 13 | M | Dalmatia | Bato | 1 | 20 | 2 | 41FE | 43EC (11) |
| 14 | N | Dacia | Decebalus | 2 | 30 | 1 | 4226 | 442E (12) |
| 15 | O | Hispania | Hasdrubal | 2 | 30 | 3 | 42C8 | 4524 (9) |
| 16 | P | Mauretania | King Juba | 2 | 40 | 2 | 42C8 | 4524 (9) |
| 17 | Q | Cyrenaica | Bocchus | 2 | 40 | 2 | 4278 | 44B2 (8) |
| 18 | R | Arabia | Mamud the Terrible | 1 | 10 | 1 | 42C8 | 4524 (9) |
| 19 | S | Mesopotamia | Horaxus | 3 | 30 | 2 | 42A0 | 44E2 (11) |
| 20 | T | Armenia | Bertonus | 1 | 20 | 1 | 42A0 | 44E2 (11) |
| 21 | U | Sicilia | Hamilcar | 1 | 40 | 1 | 42C8 | 4524 (9) |
| 22 | V | Sardinia | Darminus | 2 | 40 | 1 | 424E | 4476 (10) |
| 23 | W | Britannia | Caswallon | 4 | 10 | 1 | 41FE | 43EC (11) |
| 24 | X | Narbonensis | Galba | 0 | 10 | 1 | 4226 | 442E (12) |
| 25 | Y | Scythia | Chrysogones | 1 | 10 | 0 | 41FE | 43EC (11) |
| 26 | Z | Sarmatia | Malodor the Impaler | 1 | 10 | 1 | 4226 | 442E (12) |

## A = zone 1, Carthago: Hannibal (LEADERA.CBM, TEXTA.LZW, 13 strings)

Scout: "You arrive in Carthago, the legendary capital of the powerful Carthaginian Empire and the greatest city west of Aegyptus.  A history-making confrontation is at hand."

**state 0**:
- "Hannibal does not parley with junior officers.  Prepare for battle!"
- "The master of Carthage doesn't deal with underlings.  Prepare for battle!"
- "I beat your best generals, and now I am sent a subaltern? The war resumes."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "Hail, General! Do you seek a parley before the battle?"
  - Friendly [F] (code 2) "We come in friendship." -> 12
  - Diplomatic [D] (code 40) "Rome desires only peace between our great nations." -> mood 0: 3, mood 1: 4, mood 2: 4
  - Aggressive [A] (code 11) "Lay down your weapons." -> mood 0: 6 / weak 5, mood 1: 6 / weak 5, mood 2: 6 / weak 5

**state 2** (never reached):
- *(empty string)*

**state 3**:
- "Ho! You intend this as a joke?"
  - Friendly [F] (code 37) "I am here to make peace." -> 10
  - Diplomatic [D] (code 38) "We will try to earn your trust." -> 7
  - Offer Alliance [O] (code 10) "We want you as allies." -> 11

**state 4**:
- "As long as your legions are in Carthage, there can be no peace."
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 11
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 10
  - Offer Alliance [O] (code 6) "We offer an alliance." -> 8

**state 5**:
- "In the face of a weaker foe? Oh, I think not."
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 9
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 6**:
- "Hannibal does not surrender.  Not to Rome, not to anyone."
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 10
  - Ultimatum [U] (code 1) "Surrender or die!" -> 9
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)

**state 7**:
- "Trust? Between Rome and Carthage?"
  - Friendly [F] (code 37) "I am here to make peace." -> 10
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 11
  - Offer Alliance [O] (code 10) "We want you as allies." -> 11

**state 8**:
- "I am no politician, but I assure you Carthage will not be Rome's willing servant."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 9**:
- "Your bravado is most entertaining."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 10**:
- "Leave Carthage now, or fight us."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "Impossible.  Sound the call for battle!"
- "Farewell, Roman.  To arms!"
- "A foul Roman dream.  On with the war!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 12**:
- "Oh, I hardly think so, Roman."
  - Friendly [F] (code 8) "We should trust each other, my friend." -> 7
  - Diplomatic [D] (code 38) "We will try to earn your trust." -> 7
  - Offer Alliance [O] (code 6) "We offer an alliance." -> 8

## B = zone 2, Aegyptus: Cleopatra (LEADERB.CBM, TEXTB.LZW, 21 strings)

Scout: "You arrive in Alexandria, the fabulous capital city of the Aegyptian Empire and home to the beautiful and legendary Queen Cleopatra."

**state 0**:
- "Tell the Roman to take his tin soldiers elsewhere.  I do not have time for minor functionaries."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "So the mighty conqueror hungers for the wealth of the pharaohs."
  - Friendly [F] (code 27) "Rome can be a powerful friend." -> mood 0: 5, mood 1: 5, mood 2: 4
  - Diplomatic [D] (code 28) "Great queen, we merely wish to negotiate." -> mood 0: 5, mood 1: 5, mood 2: 6
  - Aggressive [A] (code 29) "Capitulate now, and there will be no bloodshed." -> mood 0: 7 / weak 14, mood 1: 7 / weak 14, mood 2: 7 / weak 14

**state 2**:
- "We are pleased with your offer.  You have an ally.  Dine with us tonight, and the gods will favor our alliance!"
- then: DS:4DD4 feast text, DS:4E28 bedchamber text, the bedchamber picture and the second portrait; state 15

**state 3**:
- "You ask too much.  The audience is over.  Escort the Roman to his ship!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "Ha! What does Rome offer that Cleopatra needs?"
  - Friendly [F] (code 30) "Protection, great queen.  Aegyptus has weakened, and Rome is great." -> 8
  - Diplomatic [D] (code 31) "Rome desires only peace between our great nations." -> 14
  - Offer Alliance [O] (code 34) "I can offer you an alliance with Rome." -> 11

**state 5**:
- "Your deeds reveal the truth, Roman, and the truth is that you are a terrible liar."
  - Diplomatic [D] (code 31) "Rome desires only peace between our great nations." -> 14
  - Aggressive [A] (code 29) "Capitulate now, and there will be no bloodshed." -> mood 0: 7 / weak 14, mood 1: 7 / weak 14, mood 2: 7 / weak 14
  - Ultimatum [U] (code 1) "Surrender or die!" -> 14

**state 6**:
- "Ha! To negotiate with Cleopatra requires more wits than you have, Roman!"
  - Aggressive [A] (code 32) "Take care! Arrogant queens are easily crushed!" -> mood 0: 9 / weak 14, mood 1: 9 / weak 14, mood 2: 9 / weak 14
  - Diplomatic [D] (code 33) "Indeed, we are humbled by your presence." -> 10
  - Offer Alliance [O] (code 34) "I can offer you an alliance with Rome." -> 11

**state 7**:
- "How presumptuous! The audience is over!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 8**:
- "You presume a great deal, Roman.  But what is the price for such friendships?"
  - Friendly [F] (code 36) "We should trust each other, great queen." -> 14
  - Diplomatic [D] (code 31) "Rome desires only peace between our great nations." -> 14
  - Offer Alliance [O] (code 35) "We offer an alliance, and promise protection in exchange for tribute." -> mood 0: 12, mood 1: 12, mood 2: 13 / weak 12

**state 9**:
- "Then fight us, Roman pigs!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 10**:
- "Stop simpering, Roman, and leave Alexandria at once!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "An empty alliance? Do you take me for a complete idiot? You may leave now, Roman."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 12**:
- "Ha! Your pitiful legions protect Aegyptus? Leave before I lose my temper!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 13**:
- "We will consider it.  What tribute do you ask of us?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, then the feast)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 2 (ALLIANCE, then the feast)
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

**state 14**:
- "You bore me Roman.  I grow weary of this empty chatter.  You may leave now."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 15** (after the feast and the bedchamber texts):
- "Does the mighty conqueror seek another conquest? Is the land of the Pharaohs not enough to satisfy you?"
  - Warm [W] (code 53) "My feelings for you are genuine.  Your beauty, Cleopatra, is unsurpassed throughout the world." -> 16
  - Sarcastic [S] (code 54) "How you flatter yourself, great Queen." -> 17
  - Aggressive [A] (code 55) "You are the only conquest I seek, and I never lose a battle." -> 18

**state 16**:
- "Then it is true.  The mighty conqueror hungers for the Queen of the Nile."
  - Gentle [G] (code 56) "My queen, hunger is a simple need, while love is a great gift." -> 20
  - Pushy [P] (code 57) "Romans take what is theirs.  It is that simple." -> 19
  - Rude [R] (code 58) "I grow impatient with you, Cleopatra." -> 19

**state 17** (shown, then the guards text DS:4D0C; END):
- "Flattery is beneath me, Roman, and SO ARE YOU.  Now you have my permission to LEAVE."

**state 18** (shown, then the guards text DS:4D0C; END):
- "Is that so?  Perhaps you are about to have a new experience, Roman.  GUARDS!!!"

**state 19** (shown, then the guards text DS:4D0C; END):
- "What IMPUDENCE.  This was a mistake.  GUARDS!!!"

**state 20**:
- "If it were offered, it might be a gift that would interest me."
  - Approach [A] (code 59) "Then I offer you this gift..." -> 21
  - Leave [L] (code 60) "My queen, I must go." -> 22

**state 21** (no string): the love animation 0x4852 (LOVE1), then the barge (LOVE2 frame 2) with random lightning flashes (frame 3) until a click; END.

**state 22** (no string): END.

## C = zone 3, Macedonia: Philip of Macedon (LEADERC.CBM, TEXTC.LZW, 13 strings)

Scout: "Your forces enter the realm of the Greeks and Macedonians.  The Greek era has given way to the age of Rome, but Macedon remains a powerful nation."

**state 0**:
- "Invader! Withdraw or prepare to fight!"
- "Retreat or do battle!"
- "Intruder! Our armies await you!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "Greetings, General! Would you negotiate with a Macedonian?"
  - Friendly [F] (code 8) "We should trust each other, my friend." -> 11
  - Diplomatic [D] (code 48) "Your courage and wisdom are legendary." -> mood 0: 12, mood 1: 12, mood 2: 4
  - Aggressive [A] (code 19) "Your choice is simple. Surrender with honor or die in disgrace." -> 7

**state 2**:
- "So be it.  From this day forward, Macedonia and Rome are allies."
- "It is settled.  The heirs of Romulus and of Alexander are now allies."
- "Let the world know that Rome and Macedonia stand together as allies."

**state 3**:
- "Your demands are an insult.  Prepare for battle!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "We accept your compliment."
  - Diplomatic [D] (code 43) "Rome desires only peace between our great nations." -> 5
  - Aggressive [A] (code 11) "Lay down your weapons." -> 12
  - Offer Alliance [O] (code 49) "Then we offer an alliance, and the promise of protection." -> 6

**state 5**:
- "Difficult, but perhaps possible.  What do you propose?"
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 12
  - Aggressive [A] (code 11) "Lay down your weapons." -> 12
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> mood 0: 8 / weak 9, mood 1: 8 / weak 9, mood 2: 8 / weak 9

**state 6**:
- "Your compliment was not THAT good, Roman.  Take your army and go."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 7**:
- "You are a barbarian, and there is but one way to deal with barbarians!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 8**:
- "What level of tribute do you require?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 2 (ALLIANCE, end)
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

**state 9**:
- "Your army is small.  Perhaps we should protect you."
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 10
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 10**:
- "Then fight us, Roman.  We are ready."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "Ridiculous! You can lie better than that, Roman!"
  - Friendly [F] (code 20) "I am here to make peace." -> 12
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 12
  - Offer Alliance [O] (code 10) "We want you as allies." -> 12

**state 12**:
- "Euripides said a bad beginning makes a bad ending.  Prepare to die, Roman."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

## D = zone 4, Gaul: Vercingetorix (LEADERD.CBM, TEXTD.LZW, 15 strings)

Scout: "Your legion enters the forbidding realm of the Gauls, fierce warriors known for their frenzied assaults that strike fear in the hearts of the most battle- hardened soldiers."

**state 0**:
- "I SPIT ON STINKING ROMAN DOGS.  PTOOEE!!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)
  - Ultimatum [U] (code 1) "Surrender or die!" -> 7

**state 1**:
- "WE HAVE NOT INVITED YOU HERE.  WHAT DO YOU SEEK, ROMAN?"
  - Friendly [F] (code 8) "We should trust each other, my friend." -> mood 0: 12, mood 1: 9 / weak 12, mood 2: 9
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> mood 0: 12, mood 1: 12, mood 2: 12 / weak 9
  - Aggressive [A] (code 11) "Lay down your weapons." -> 14

**state 2**:
- "THEN WE SUBMIT TO YOUR DEMANDS.  FROM THIS DAY FORWARD, WE OBEY THE WILL OF ROME.  YOU HAVE MY WORD."
- "WE ARE YOUR SUBJECTS."
- "THANK YOU, MIGHTY ONE.  WE OBEY THE WILL OF ROME."

**state 3**:
- "I GROW TIRED OF YOU, ROMAN.  YOUR PITIFUL LEGIONS ARE NO MATCH FOR THE GAULS."
- "ROMAN, YOU FILL ME WITH DISGUST.  NOW YOU WILL DIE!!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "THEN WE GREET YOU AS FRIENDS."
  - Diplomatic [D] (code 3) "Your courage and wisdom are legendary." -> mood 0: 12, mood 1: 8 / weak 12, mood 2: 8
  - Friendly [F] (code 8) "We should trust each other, my friend." -> mood 0: 12, mood 1: 9 / weak 12, mood 2: 9
  - Offer Alliance [O] (code 10) "We want you as allies." -> 5

**state 5**:
- "ROMAN DOGS! WE SEE THROUGH YOUR LIES!"
  - Go to War [G] (code 12) "Die, barbarian pig!" END (battle B06E)
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> mood 0: 10 / weak 11, mood 1: 10 / weak 11, mood 2: 10 / weak 11
  - Ultimatum [U] (code 1) "Surrender or die!" -> 7

**state 6**:
- "GAUL WILL NEVER JOIN YOUR EMPIRE."
  - Ultimatum [U] (code 1) "Surrender or die!" -> 7
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 7**:
- "WE WILL NEVER SUBMIT! IF THAT IS YOUR WISH, TASTE THE FURY OF GAUL!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 8**:
- "EMPTY PRAISE.  YOU TRY OUR PATIENCE, ROMAN."
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> mood 0: 10 / weak 11, mood 1: 10 / weak 11, mood 2: 10 / weak 11
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)
  - Offer Alliance [O] (code 6) "We offer an alliance." -> mood 0: 3, mood 1: 3 / weak 6, mood 2: 13 / weak 6

**state 9**:
- "WHAT TERMS DO YOU OFFER?"
  - Ultimatum [U] (code 1) "Surrender or die!" -> 7
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> mood 0: 10 / weak 11, mood 1: 10 / weak 11, mood 2: 10 / weak 11
  - Diplomatic [D] (code 9) "Our terms are extremely generous." -> 3

**state 10**:
- "ROMAN DOG! YOU WILL PAY FOR THIS INSULT WITH YOUR LIFE!"
- "THEN DIE LIKE A ROMAN!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "LET THE GODS OF WAR DECIDE!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 12**:
- "YOUR LIES ARE TOO OBVIOUS, ROMAN."
  - Go to War [G] (code 12) "Die, barbarian pig!" END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)
  - Ultimatum [U] (code 1) "Surrender or die!" -> 7

**state 13**:
- "WHAT LEVEL OF TRIBUTE WILL YOU DEMAND?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 3
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

**state 14**:
- "GAUL SURRENDER TO ROMANS? NEVER!"
  - Friendly [F] (code 2) "We come in friendship." -> mood 0: 5, mood 1: 4 / weak 5, mood 2: 4
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> mood 0: 10 / weak 11, mood 1: 10 / weak 11, mood 2: 10 / weak 11
  - Offer Alliance [O] (code 10) "We want you as allies." -> 5

## E = zone 5, Parthia: Darius the Great (LEADERE.CBM, TEXTE.LZW, 13 strings)

Scout: "The easternmost reaches of your maps show the border of the vast Parthian Empire, a prize beyond compare.  As your troops cross into Parthia, an army intercepts them."

**state 0**:
- "Rome sends lackeys to the Parthian king? We are insulted."
- "Go, sluggard.  The king of Parthia has nothing to say to inferiors."
- "Sound the attack! Pray to your gods for mercy!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "Hail! Something to say before we destroy your legions?"
  - Friendly [F] (code 2) "We come in friendship." -> 12
  - Diplomatic [D] (code 40) "Rome desires only peace between our great nations." -> mood 0: 3, mood 1: 4, mood 2: 4
  - Aggressive [A] (code 11) "Lay down your weapons." -> mood 0: 6 / weak 5, mood 1: 6 / weak 5, mood 2: 6 / weak 5

**state 2** (never reached):
- *(empty string)*

**state 3**:
- "I did not know Romans could jest!"
  - Friendly [F] (code 37) "I am here to make peace." -> 10
  - Diplomatic [D] (code 38) "We will try to earn your trust." -> 7
  - Offer Alliance [O] (code 10) "We want you as allies." -> 11

**state 4**:
- "Withdraw your legions, and then there can be peace."
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 11
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 10
  - Offer Alliance [O] (code 6) "We offer an alliance." -> 8

**state 5**:
- "When we are the stronger army? Oh, come now!"
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 9
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 6**:
- "Parthians never surrender, especially not to Rome!"
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 10
  - Ultimatum [U] (code 1) "Surrender or die!" -> 9
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)

**state 7**:
- "Trust Rome? Are you sober?"
  - Friendly [F] (code 37) "I am here to make peace." -> 10
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 11
  - Offer Alliance [O] (code 10) "We want you as allies." -> 11

**state 8**:
- "The lion of Parthia will never serve the jackal of Rome!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 9**:
- "Your posturing amuses us."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 10**:
- "Withdraw at once or the king will have your head on a pike!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "You are mad! Begin the the attack!"
- "Never, son of a pig! Attack them now!"
- "Out of the question.  Prepare to die."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 12**:
- "Oh, that hardly sounds likely."
  - Friendly [F] (code 8) "We should trust each other, my friend." -> 7
  - Diplomatic [D] (code 38) "We will try to earn your trust." -> 7
  - Offer Alliance [O] (code 6) "We offer an alliance." -> 8

## F = zone 6, Syria: Antiochus (LEADERF.CBM, TEXTF.LZW, 14 strings)

Scout: "Your scouts report that Syria is a land of wise merchants and seafarers.  In a desert valley along the coastal plain, your forces are confronted by the Syrian army."

**state 0**:
- "Be warned, Roman.  Withdraw or risk attack."
- "Be gone, intruder.  It is not too late to flee."
- "Syria will not suffer invaders! Go or fight!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "If you have something to say, be quick about it."
  - Friendly [F] (code 2) "We come in friendship." -> mood 0: 12 / weak 13, mood 1: 12 / weak 13, mood 2: 12 / weak 13
  - Diplomatic [D] (code 3) "Your courage and wisdom are legendary." -> 9
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> 10

**state 2**:
- "That is reasonable.  You have an ally."
- "A fair bargain.  Syria is your ally."
- "That we can accept.  From this day, we are allies."

**state 3**:
- "Our people would starve! The answer is no."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "What does Rome mean by this word friendship?"
  - Friendly [F] (code 8) "We should trust each other, my friend." -> 6
  - Diplomatic [D] (code 43) "Rome desires only peace between our great nations." -> 6
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> mood 0: 8, mood 1: 8, mood 2: 7

**state 5**:
- "I think not, Roman.  You seek slaves, not allies."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 6**:
- "You speak in lofty terms, yet the words say nothing.  Leave us."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 7**:
- "We are inclined to believe you.  What tribute will you demand?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 3
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

**state 8**:
- "Rome's idea of protection holds no interest for us.  Take your men and go!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 9**:
- "Flattery! Tell us what you want, Roman."
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 6
  - Aggressive [A] (code 41) "Your choice is simple. Surrender with honor or die in disgrace." -> 11
  - Offer Alliance [O] (code 10) "We want you as allies." -> 5

**state 10**:
- "Syria never bows to aggressors! Withdraw or fight!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "Syrians do not surrender.  If you attack us, we will fight."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 12**:
- "Forgive us but we do not believe you."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 13**:
- "We admire your courage."
  - Friendly [F] (code 42) "Rome can be a powerful friend." -> 4
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 6
  - Offer Alliance [O] (code 10) "We want you as allies." -> 5

## G = zone 7, Thracia: Thorax of Thrace (LEADERG.CBM, TEXTG.LZW, 13 strings)

Scout: "Your scouts report Thracia is a mountainous province with fierce, intractable leaders and skilled warriors.  Invaders fare poorly here."

**state 0**:
- "All of Thracia will rise to expel you!"
- "Roman vermin! We shall crush you!"
- "Accursed one! Your fate is upon you!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "Hail, Roman.  What have you to offer Thracia?"
  - Friendly [F] (code 8) "We should trust each other, my friend." -> 11
  - Diplomatic [D] (code 48) "Your courage and wisdom are legendary." -> mood 0: 12, mood 1: 12, mood 2: 4
  - Aggressive [A] (code 19) "Your choice is simple. Surrender with honor or die in disgrace." -> 7

**state 2**:
- "Upon our honor, Thracia is your ally forever."
- "The bond is sealed; we are now the faithful ally of Rome."
- "We are your ally.  Your friends are our friends, your foes, our foes."

**state 3**:
- "Arrogant dog! Gather your pitiful legion to die."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "We accept your gracious words."
  - Diplomatic [D] (code 43) "Rome desires only peace between our great nations." -> 5
  - Aggressive [A] (code 11) "Lay down your weapons." -> 12
  - Offer Alliance [O] (code 49) "Then we offer an alliance, and the promise of protection." -> 6

**state 5**:
- "Hard, but possible.  How would you go about it?"
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 12
  - Aggressive [A] (code 11) "Lay down your weapons." -> 12
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> mood 0: 8 / weak 9, mood 1: 8 / weak 9, mood 2: 8 / weak 9

**state 6**:
- "No words of praise will excuse that.  Your army must leave."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 7**:
- "You are little better than a savage.  Thracians meet savagery with savagery!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 8**:
- "How much tribute do you want?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 2 (ALLIANCE, end)
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

**state 9**:
- "With so few soldiers? Perhaps WE should be the protectors."
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 10
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 10**:
- "Then draw your weapons!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "Absurd! Can you lie no better than that?"
  - Friendly [F] (code 20) "I am here to make peace." -> 12
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 12
  - Offer Alliance [O] (code 10) "We want you as allies." -> 12

**state 12**:
- "We have beaten far better warriors than you.  Now you die!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

## H = zone 8, Pontus: Mithridates (LEADERH.CBM, TEXTH.LZW, 14 strings)

Scout: "Pontus is a small region along the southern coast of the Black Sea.  A wealthy kingdom, Pontus can afford to field a well-supplied army."

**state 0**:
- "You are not welcome.  Flee while you may."
- "If you remain in Pontus, war is certain."
- "Retreat or fight us, but choose now!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "Say what you have come to say and be gone."
  - Friendly [F] (code 2) "We come in friendship." -> mood 0: 12 / weak 13, mood 1: 12 / weak 13, mood 2: 12 / weak 13
  - Diplomatic [D] (code 3) "Your courage and wisdom are legendary." -> 9
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> 10

**state 2**:
- "Your terms are fair.  We are your ally."
- "Most agreeable terms.  We are allies."
- "Your generosity suprises us.  Agreed."

**state 3**:
- "Outrageous! We refuse."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "Rome sometimes treats her friends bitterly."
  - Friendly [F] (code 8) "We should trust each other, my friend." -> 6
  - Diplomatic [D] (code 43) "Rome desires only peace between our great nations." -> 6
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> mood 0: 8, mood 1: 8, mood 2: 7

**state 5**:
- "Impossible.  Rome says 'ally' when she means 'slave'."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 6**:
- "Your high-flown language is meaningless.  Be gone."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 7**:
- "Your words ring true.  What tribute will suffice?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 2 (ALLIANCE, end)
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

**state 8**:
- "Rome would 'protect' us into chains and bondage.  Go now!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 9**:
- "Speak plainly, Roman, what do you want?"
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 6
  - Aggressive [A] (code 41) "Your choice is simple. Surrender with honor or die in disgrace." -> 11
  - Offer Alliance [O] (code 10) "We want you as allies." -> 5

**state 10**:
- "We laugh at your bluffs.  Go or die!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "We will fight if you attack.  We are not cowards."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 12**:
- "Some of us think you lie."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 13**:
- "Your bravery is great."
  - Friendly [F] (code 42) "Rome can be a powerful friend." -> 4
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 6
  - Offer Alliance [O] (code 10) "We want you as allies." -> 5

## I = zone 9, Asia: Attalus (LEADERI.CBM, TEXTI.LZW, 14 strings)

Scout: "As your legionnaires march into Asia, her armies assemble on a plain by the sea.  You stand at the gateway to the east, with the destiny of Rome in your hands."

**state 0**:
- "We won't warn you twice; go or fight us!"
- "Rome is not welcome here.  Go or face war!"
- "Asians are warriors.  Must we prove it?"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "Speak your speech and then leave us."
  - Friendly [F] (code 2) "We come in friendship." -> mood 0: 12 / weak 13, mood 1: 12 / weak 13, mood 2: 12 / weak 13
  - Diplomatic [D] (code 3) "Your courage and wisdom are legendary." -> 9
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> 10

**state 2**:
- "Fairer terms than we expected.  Agreed."
- "You are just, Roman.  We are allies."
- "That is acceptable.  Asia is your ally."

**state 3** (never reached):
- "Never! You would leave us with nothing!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "What does friendship with Rome mean?"
  - Friendly [F] (code 8) "We should trust each other, my friend." -> 6
  - Diplomatic [D] (code 43) "Rome desires only peace between our great nations." -> 6
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> mood 0: 8, mood 1: 8, mood 2: 7

**state 5**:
- "Never.  Your ally today, your slave tomorrow."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 6**:
- "Fair words, but empty ones.  Go!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 7**:
- "We feel you are truthful.  What tribute do you wish?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 2 (ALLIANCE, end)
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 2 (ALLIANCE, end)

**state 8**:
- "But who would protect us from you? Withdraw your legions now!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 9**:
- "Just say what you want, Roman."
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 6
  - Aggressive [A] (code 41) "Your choice is simple. Surrender with honor or die in disgrace." -> 11
  - Offer Alliance [O] (code 10) "We want you as allies." -> 5

**state 10**:
- "Attack us at your peril! Asians fear no one!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "Threats do not impress us! We will resist your legions!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 12**:
- "Pardon us if we think you are a liar."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 13**:
- "Your bravery is proven."
  - Friendly [F] (code 42) "Rome can be a powerful friend." -> 4
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 6
  - Offer Alliance [O] (code 10) "We want you as allies." -> 5

## J = zone 10, Cilicia: Absolna the Rotten (LEADERJ.CBM, TEXTJ.LZW, 13 strings)

Scout: "You have reached Cilicia, home of the Cilician pirates.  They are salacious brigands, desperate scoundrels, and the scourge of Mare Nostrum.  The scouts' advice is simple: SQUASH THEM!"

**state 0**:
- "The king of Cilicia will not parley with an underling."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "Roman leader! What are your intentions?"
  - Friendly [F] (code 2) "We come in friendship." -> mood 0: 12, mood 1: 11, mood 2: 11
  - Diplomatic [D] (code 45) "We seek to negotiate with you." -> 4
  - Aggressive [A] (code 44) "Lay down your weapons." -> 5

**state 2**:
- "Then let us be allies! Pirates and Romans belong together!"

**state 3**:
- "Your greed exceeds even mine! Who is the true pirate here?"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "Of course, of course.  And THEN you will attack.  But why waste time? Fight us now!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 5**:
- "Before we fight? The pirates of Cilicia do not fear anyone.  Especially Romans!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 6**:
- "We trust no one.  Speak plainly and tell us what you want."
  - Friendly [F] (code 47) "Rome can be a powerful friend." -> 9
  - Diplomatic [D] (code 46) "Hear me, wise and honorable chieftain." -> 7
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> 10

**state 7**:
- "Blowhard! A Roman blowhard!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 8**:
- "Indeed! And what will it cost, my friend?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 3
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

**state 9**:
- "Talk, talk and more talk! Let us fight.  It would be more...  interesting."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 10**:
- "Enough talk.  You bore me, Roman.  If you want war, you may have it."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "Of course, of course.  EVERYONE wants to be friends with dishonorable pirates."
  - Friendly [F] (code 47) "Rome can be a powerful friend." -> 9
  - Diplomatic [D] (code 45) "We seek to negotiate with you." -> 4
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> 10

**state 12**:
- "In that case, welcome.  You are among friends."
  - Friendly [F] (code 8) "We should trust each other, my friend." -> 6
  - Diplomatic [D] (code 46) "Hear me, wise and honorable chieftain." -> 7
  - Offer Alliance [O] (code 17) "We come in peace and seek an ally." -> 8

## K = zone 11, Alpes: Vindelic (LEADERK.CBM, TEXTK.LZW, 15 strings)

Scout: "You lead your legion into the mountainous Alpes north of Italia.  At the edge of the forest your scouts cry out the dreaded word: BARBARIANS!"

**state 0**:
- "THE TRIBES OF THESE MOUNTAINS NEED YOU NOT."
- "GO AWAY, WARRIORS FROM THE SOUTH."
- "LEAVE THE WAY YOU CAME, FOR YOU ARE NOT WELCOME HERE."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)
  - Ultimatum [U] (code 1) "Surrender or die!" -> 7

**state 1**:
- "WELCOME TO THE MOUNTAINS, GREAT WARRIOR."
  - Friendly [F] (code 8) "We should trust each other, my friend." -> mood 0: 12, mood 1: 9 / weak 12, mood 2: 9
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> mood 0: 12, mood 1: 12, mood 2: 12 / weak 9
  - Aggressive [A] (code 11) "Lay down your weapons." -> 14

**state 2**:
- "WE WILL AGREE TO THIS, AT LEAST FOR NOW."
- "YOU HONOR US, GREAT WARRIOR.  THE GODS WILL FAVOR THIS TREATY."
- "WE JOIN YOUR TRIBE IN HOPE THERE IS BOOTY TO WIN!"

**state 3**:
- "YOU ANGER ME, WARRIOR.  WOULD YOU FIGHT US?"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "GREAT WARRIOR, YOUR WORDS HAVE BEEN HEARD."
  - Diplomatic [D] (code 3) "Your courage and wisdom are legendary." -> mood 0: 12, mood 1: 8 / weak 12, mood 2: 8
  - Friendly [F] (code 8) "We should trust each other, my friend." -> mood 0: 12, mood 1: 9 / weak 12, mood 2: 9
  - Offer Alliance [O] (code 10) "We want you as allies." -> 5

**state 5**:
- "TRUTH IS FLEETING, GREAT WARRIOR, AND WE KNOW YOU NOT."
  - Go to War [G] (code 12) "Die, barbarian pig!" END (battle B06E)
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> mood 0: 10 / weak 11, mood 1: 10 / weak 11, mood 2: 10 / weak 11
  - Ultimatum [U] (code 1) "Surrender or die!" -> 7

**state 6**:
- "OUR MOUNTAINS HAVE MANY SECRET PLACES."
  - Ultimatum [U] (code 1) "Surrender or die!" -> 7
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 7**:
- "WE WOULD FIGHT YOU FIRST!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 8**:
- "I MUST BE CAREFUL.  STILL, I WILL LISTEN."
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> mood 0: 10 / weak 11, mood 1: 10 / weak 11, mood 2: 10 / weak 11
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)
  - Offer Alliance [O] (code 6) "We offer an alliance." -> mood 0: 3, mood 1: 3 / weak 6, mood 2: 13 / weak 6

**state 9**:
- "WHAT MUST WE GIVE UP?"
  - Ultimatum [U] (code 1) "Surrender or die!" -> 7
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> mood 0: 10 / weak 11, mood 1: 10 / weak 11, mood 2: 10 / weak 11
  - Diplomatic [D] (code 9) "Our terms are extremely generous." -> 3

**state 10**:
- "FIGHT US, WARRIOR FROM THE SOUTH!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "OUR TRIBES ARE NOW AT WAR!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 12**:
- "YOU WOULD LIE TO US? ATTACK THEM!"
  - Go to War [G] (code 12) "Die, barbarian pig!" END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)
  - Ultimatum [U] (code 1) "Surrender or die!" -> 7

**state 13**:
- "THEN YOU WANT TRIBUTE.  WILL IT BE HIGH?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 3
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

**state 14**:
- "WE HAVE NOTHING MORE TO SAY TO YOU, WARRIOR."
  - Friendly [F] (code 2) "We come in friendship." -> mood 0: 5, mood 1: 4 / weak 5, mood 2: 4
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> mood 0: 10 / weak 11, mood 1: 10 / weak 11, mood 2: 10 / weak 11
  - Offer Alliance [O] (code 10) "We want you as allies." -> 5

## L = zone 12, Germania: Arminius (LEADERL.CBM, TEXTL.LZW, 15 strings)

Scout: "Before you stretches the dense forests of Germania, where live uncounted barbarian tribes who fight with reckless abandon."

**state 0**:
- "THE TRIBES WILL NOT HEAR YOUR LIES.  TURN BACK OR PREPARE TO DIE."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)
  - Ultimatum [U] (code 1) "Surrender or die!" -> 7

**state 1**:
- "GO HOME, ROMAN.  YOU ARE NOT WELCOME IN OUR LAND."
  - Friendly [F] (code 8) "We should trust each other, my friend." -> mood 0: 12, mood 1: 9 / weak 12, mood 2: 9
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> mood 0: 12, mood 1: 12, mood 2: 12 / weak 9
  - Aggressive [A] (code 11) "Lay down your weapons." -> 14

**state 2**:
- "YOU LEAVE US NO CHOICE.  WE ARE FORCED TO SURRENDER."
- "THE GODS DO NOT WISH US TO FIGHT THIS DAY.  WE OBEY YOUR WILL."
- "ROMAN LAW IS OUR LAW.  WE ARE NOW YOUR ALLIES."

**state 3**:
- "ROMAN SMELL LIKE ELEPHANT."
- "ROMAN LOOK LIKE WARTHOG."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "THAT REMAINS TO BE SEEN, ROMAN."
  - Diplomatic [D] (code 3) "Your courage and wisdom are legendary." -> mood 0: 12, mood 1: 8 / weak 12, mood 2: 8
  - Friendly [F] (code 8) "We should trust each other, my friend." -> mood 0: 12, mood 1: 9 / weak 12, mood 2: 9
  - Offer Alliance [O] (code 10) "We want you as allies." -> 5

**state 5**:
- "YOUR WORDS HAVE NOT THE RING OF TRUTH."
  - Go to War [G] (code 12) "Die, barbarian pig!" END (battle B06E)
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> mood 0: 10 / weak 11, mood 1: 10 / weak 11, mood 2: 10 / weak 11
  - Ultimatum [U] (code 1) "Surrender or die!" -> 7

**state 6**:
- "THE TRIBES ARE STRONG.  WE HAVE NO NEED OF ROME."
  - Ultimatum [U] (code 1) "Surrender or die!" -> 7
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 7**:
- "NEVER! THE TRIBES HAVE NEVER TASTED DEFEAT.  WE WILL FIGHT!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 8**:
- "YOU HAVE TREATED OTHERS WELL.  WE WILL LISTEN."
  - Friendly [F] (code 8) "We should trust each other, my friend." -> mood 0: 12, mood 1: 9 / weak 12, mood 2: 9
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> mood 0: 10 / weak 11, mood 1: 10 / weak 11, mood 2: 10 / weak 11
  - Offer Alliance [O] (code 6) "We offer an alliance." -> mood 0: 3, mood 1: 3 / weak 6, mood 2: 13 / weak 6

**state 9**:
- "WHAT ARE YOUR TERMS?"
  - Ultimatum [U] (code 1) "Surrender or die!" -> 7
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> mood 0: 10 / weak 11, mood 1: 10 / weak 11, mood 2: 10 / weak 11
  - Diplomatic [D] (code 9) "Our terms are extremely generous." -> 3

**state 10**:
- "THEN WE ARE AT WAR.  THIS BATTLE WILL BE YOUR LAST!"
- "PREPARE TO DIE, ROMAN SCUM!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "YOU ARE ABOUT TO DIE! GATHER THE TRIBES.  SOUND THE CALL TO BATTLE!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 12**:
- "LIES! YOU INSULT US WITH YOUR LIES!"
  - Go to War [G] (code 12) "Die, barbarian pig!" END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)
  - Ultimatum [U] (code 1) "Surrender or die!" -> 7

**state 13**:
- "YOU WILL DEMAND TRIBUTE.  HOW MUCH?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 3
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

**state 14**:
- "YOU MAKE US LAUGH, ROMAN."
  - Friendly [F] (code 2) "We come in friendship." -> mood 0: 5, mood 1: 4 / weak 5, mood 2: 4
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> mood 0: 10 / weak 11, mood 1: 10 / weak 11, mood 2: 10 / weak 11
  - Offer Alliance [O] (code 10) "We want you as allies." -> 5

## M = zone 13, Dalmatia: Bato (LEADERM.CBM, TEXTM.LZW, 13 strings)

Scout: "Your scouts report that Dalmatia will not be an easy conquest.  Laced with forested mountain ranges, this rugged land seldom falls to invaders and is difficult to rule effectively."

**state 0**:
- "Be gone, intruder! Leave or know our wrath!"
- "Are you here to die among strangers, or will you heed my warning and go?"
- "Flee, unwelcome one, or die where you stand!"
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 11
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "Who is this fool who enters Dalmatia unbidden?"
  - Offer Alliance [O] (code 17) "We come in peace and seek an ally." -> mood 0: 4, mood 1: 4, mood 2: 5
  - Diplomatic [D] (code 18) "Hear me, wise and honorable chieftain." -> mood 0: 7, mood 1: 7, mood 2: 6
  - Aggressive [A] (code 19) "Your choice is simple. Surrender with honor or die in disgrace." -> mood 0: 8, mood 1: 8, mood 2: 9

**state 2**:
- "So let us be allies.  Perhaps it will even last beyond sundown."
- "We are allies, but I warn you.  Treat Dalmatia fairly."
- "In my eyes an alliance is good.  May all Dalmatians view Rome as a friend."

**state 3**:
- "Never! Let fate resolve this in blood!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "Allies? Explain, for all that I believe you."
  - Diplomatic [D] (code 23) "We will try to earn your trust." -> 11
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 5**:
- "You wish to be our ally? When Romans smile, 'tis only to bare their fangs."
  - Friendly [F] (code 20) "I am here to make peace." -> 11
  - Diplomatic [D] (code 21) "Forgive our arrogance, honorable chieftain." -> 11
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> 10

**state 6**:
- "Are all Roman embassies armed?"
  - Friendly [F] (code 20) "I am here to make peace." -> 11
  - Diplomatic [D] (code 21) "Forgive our arrogance, honorable chieftain." -> 11
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> 10

**state 7**:
- "You lie! Now be prey for our swords!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 8**:
- "Words are dust.  Our swords stand firm, but our honor often bends."
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 11
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> mood 0: 11 / weak 3, mood 1: 11 / weak 3, mood 2: 11 / weak 3
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> 10

**state 9**:
- "We crave peace, but spurn slavery.  We draw swords only to fight, never to salute!"
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 11
  - Aggressive [A] (code 24) "That is the choice we offer, and we will not make it again." -> 11
  - Offer Alliance [O] (code 25) "Then we offer an alliance, and the promise of protection." -> mood 0: 10, mood 1: 10, mood 2: 12 / weak 10

**state 10**:
- "Are we dogs who need Roman protection? See instead how we bite!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "Enough talk, the fray is begun.  Men of Dalmatia can do no other."
- "Idle promises.  Take them and be gone!"
- "I wait for a reply, yet the dog is silent.  Bow down, Roman!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 12**:
- "What tribute do you require?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 2 (ALLIANCE, end)
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

## N = zone 14, Dacia: Decebalus (LEADERN.CBM, TEXTN.LZW, 13 strings)

Scout: "Your troops cross into Dacia.  The warriors of Dacia are among the fiercest in the known world and the land is unfriendly to any intruder, especially the emissaries of Rome."

**state 0**:
- "We won't deal with women.  Go bid your menfolk farewell."
- "They send a eunuch to parlay with warriors.  Prepare to die!"
- "Pray you're a better man than you appear.  Now we attack!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "Hail, Roman.  You desire a meeting before you die?"
  - Friendly [F] (code 2) "We come in friendship." -> 12
  - Diplomatic [D] (code 40) "Rome desires only peace between our great nations." -> mood 0: 3, mood 1: 4, mood 2: 4
  - Aggressive [A] (code 11) "Lay down your weapons." -> mood 0: 6 / weak 5, mood 1: 6 / weak 5, mood 2: 6 / weak 5

**state 2** (never reached):
- *(empty string)*

**state 3**:
- "You jest poorly."
  - Friendly [F] (code 37) "I am here to make peace." -> 10
  - Diplomatic [D] (code 38) "We will try to earn your trust." -> 7
  - Offer Alliance [O] (code 10) "We want you as allies." -> 11

**state 4**:
- "Peace will come when you are gone...  or dead!"
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 11
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 10
  - Offer Alliance [O] (code 6) "We offer an alliance." -> 8

**state 5**:
- "Even though we could crush you? Ha!"
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 9
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 6**:
- "Only weaklings surrender.  Dacians aren't weaklings!"
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 10
  - Ultimatum [U] (code 1) "Surrender or die!" -> 9
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)

**state 7**:
- "I'd sooner trust a wolf."
  - Friendly [F] (code 37) "I am here to make peace." -> 10
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 11
  - Offer Alliance [O] (code 10) "We want you as allies." -> 11

**state 8**:
- "We would rather boil in hell than serve you!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 9**:
- "Your bragging is stupid."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 10**:
- "I'll warn you no more.  Withdraw!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "Laughable! Today you die!"
- "Make peace with your gods, dog!"
- "War is my answer!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 12**:
- "That can never be, Roman!"
  - Friendly [F] (code 8) "We should trust each other, my friend." -> 7
  - Diplomatic [D] (code 38) "We will try to earn your trust." -> 7
  - Offer Alliance [O] (code 6) "We offer an alliance." -> 8

## O = zone 15, Hispania: Hasdrubal (LEADERO.CBM, TEXTO.LZW, 12 strings)

Scout: "Dry and dusty, the region called Hispania is part of the Carthaginian Empire and the armies of Carthage stand ready for battle.  If it comes to a fight, your legionnaires truly will be tested here."

**state 0**:
- "Dog of Rome! We shall rip out your lying tongue!"
- "Invader! We shall cut out your black heart!"
- "Pig! I'll have your head on a pike at my gate!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "You may address us, envoy.  But be brief."
  - Friendly [F] (code 2) "We come in friendship." -> 10
  - Diplomatic [D] (code 46) "Hear me, wise and honorable chieftain." -> 11
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 4

**state 2**:
- "Our alliance is forged.  We are exposed on four coasts, and need your protection."
- "Then Hispania is Rome's ally.  May your power protect us from a host of foes."
- "This alliance is acceptable.  We had grown weary of Carthaginian rule."

**state 3** (never reached):
- "We will not suffer such thievery!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "Leech! If you thirst for blood, drink your own as we spill it!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 5**:
- "Your career as a diplomat has come to an end.  Leave us."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 6**:
- "Your diplomat's tongue turns to threats!"
  - Diplomatic [D] (code 45) "We seek to negotiate with you." -> 8
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 4
  - Offer Alliance [O] (code 52) "We offer an alliance, and promise protection in exchange for tribute." -> 9

**state 7**:
- "It is unwise to make such demands so far from home.  Return there or die."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 8**:
- "We are not idiots.  Be gone, Roman!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 9**:
- "What tribute is required?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 2 (ALLIANCE, end)
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 2 (ALLIANCE, end)

**state 10**:
- "You know that cannot be.  Put on yet another face."
  - Diplomatic [D] (code 3) "Your courage and wisdom are legendary." -> 5
  - Aggressive [A] (code 51) "Submit to the power of Rome!" -> 5
  - Offer Alliance [O] (code 50) "We offer an alliance." -> 5

**state 11**:
- "Trying your hand at diplomacy? Ha! Go on, go on."
  - Diplomatic [D] (code 3) "Your courage and wisdom are legendary." -> 5
  - Aggressive [A] (code 11) "Lay down your weapons." -> mood 0: 7, mood 1: 6 / weak 7, mood 2: 6 / weak 7
  - Offer Alliance [O] (code 50) "We offer an alliance." -> 5

## P = zone 16, Mauretania: King Juba (LEADERP.CBM, TEXTP.LZW, 12 strings)

Scout: "Your scouts report that Mauretania is a rocky, desert region.  The cities are confined to the coastal areas, and the leaders may not be friendly."

**state 0**:
- "Serpent! Your bones shall bleach in the wilderness!"
- "Your death shall come quickly as the cold of night!"
- "Devil! The desert shall drink deeply of your blood!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "Speak, but try not our patience."
  - Friendly [F] (code 2) "We come in friendship." -> 10
  - Diplomatic [D] (code 46) "Hear me, wise and honorable chieftain." -> 11
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 4

**state 2**:
- "We accept you as an ally.  The desert vastness is full of peril, we need your protection."
- "So is our alliance sealed.  Your might combined with ours assures our safety."
- "With your strength on our side, our enemies will fear us.  We agree to this alliance."

**state 3**:
- "Extortion! We'd sooner die than pay!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "Honorless viper! Your yen to conquer us is plain.  Attack when ye will!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 5**:
- "There is too much virtue in this tent for your seductions to work.  Be gone."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 6**:
- "The diplomat becomes a scorpion! But can you sting?"
  - Diplomatic [D] (code 45) "We seek to negotiate with you." -> 8
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 4
  - Offer Alliance [O] (code 52) "We offer an alliance, and promise protection in exchange for tribute." -> 9

**state 7**:
- "Bold demands from so small a force!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 8**:
- "You take us too lightly, Roman.  Go!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 9**:
- "What tribute will suffice?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 2 (ALLIANCE, end)
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

**state 10**:
- "Should we be insulted or amused? Pray continue."
  - Diplomatic [D] (code 3) "Your courage and wisdom are legendary." -> 5
  - Aggressive [A] (code 51) "Submit to the power of Rome!" -> 5
  - Offer Alliance [O] (code 50) "We offer an alliance." -> 5

**state 11**:
- "So, you use the diplomat's seductive tone.  Proceed."
  - Diplomatic [D] (code 3) "Your courage and wisdom are legendary." -> 5
  - Aggressive [A] (code 11) "Lay down your weapons." -> mood 0: 7, mood 1: 6 / weak 7, mood 2: 6 / weak 7
  - Offer Alliance [O] (code 50) "We offer an alliance." -> 5

## Q = zone 17, Cyrenaica: Bocchus (LEADERQ.CBM, TEXTQ.LZW, 13 strings)

Scout: "Your legion arrives in Cyrenaica, a desert region to the west of Aegyptus.  While it remains under the influence of the Aegyptian Empire, Cyrenaica may be hostile to Rome."

**state 0**:
- "The king of Cyrenaica does not make deals with lackeys."
- "Kings do not negotiate with peasants."
- "A king negotiate with a slave? Out of the question!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "What are your true goals, noble Roman?"
  - Friendly [F] (code 2) "We come in friendship." -> mood 0: 12, mood 1: 11, mood 2: 11
  - Diplomatic [D] (code 45) "We seek to negotiate with you." -> 4
  - Aggressive [A] (code 44) "Lay down your weapons." -> 5

**state 2**:
- "We will join you, for wolves belong in packs."
- "Rome lives by greed and violence.  We are meant to be allies!"
- "If we are to swim together, Roman shark, let it be as allies."

**state 3**:
- "You radiate with greed.  Admirable!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "And when our back was turned, your sword would find it.  Why not fight us now?"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 5**:
- "We may be thieves, but at least we are brave, unlike Romans!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 6**:
- "Our distrust is mutual.  Just say what you want."
  - Friendly [F] (code 47) "Rome can be a powerful friend." -> 9
  - Diplomatic [D] (code 46) "Hear me, wise and honorable chieftain." -> 7
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> 10

**state 7**:
- "You brag so very well."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 8**:
- "And what would the cost be?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 2 (ALLIANCE, end)
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

**state 9**:
- "I weary of this chatter.  Wouldn't a good fight be more stimulating?"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 10**:
- "You have grown tiresome, Roman.  If you want war, so be it."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "Naturally.  Who wouldn't want rapacious pirates as allies?"
  - Friendly [F] (code 47) "Rome can be a powerful friend." -> 9
  - Diplomatic [D] (code 45) "We seek to negotiate with you." -> 4
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> 10

**state 12**:
- "Welcome then, fellow brigand.  We are as brothers."
  - Friendly [F] (code 8) "We should trust each other, my friend." -> 6
  - Diplomatic [D] (code 46) "Hear me, wise and honorable chieftain." -> 7
  - Offer Alliance [O] (code 17) "We come in peace and seek an ally." -> 8

## R = zone 18, Arabia: Mamud the Terrible (LEADERR.CBM, TEXTR.LZW, 12 strings)

Scout: "The deserts of Arabia stretch before you, dotted only occasionally with an oasis or waterhole.  This land has little to offer Romans save death and despair."

**state 0**:
- "Foul intruder! We will paint the desert with your blood!"
- "Your blood will seep into the desert sands ere sundown!"
- "Swine! Vultures shall feast upon your bloated corpse!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "We will admit this envoy of Rome to our presence.  But only for a moment."
  - Friendly [F] (code 2) "We come in friendship." -> 10
  - Diplomatic [D] (code 46) "Hear me, wise and honorable chieftain." -> 11
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 4

**state 2**:
- "We will grant this alliance, for our enemies are many and we are but few."
- "Our people are few and surrounded by foes.  An alliance is necessary."
- "We are a small nation encircled by jackals.  This alliance shall protect us."

**state 3**:
- "Robbery! You will have to kill us first!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "You are a snake! If it is a fight you want, then you will have it!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 5**:
- "Oh, most entertaining indeed! A pity the entertainment must end.  The audience is over."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 6**:
- "Ah, the entertainment is over.  The diplomat has left."
  - Diplomatic [D] (code 45) "We seek to negotiate with you." -> 8
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 4
  - Offer Alliance [O] (code 52) "We offer an alliance, and promise protection in exchange for tribute." -> 9

**state 7**:
- "You are too weak to make such demands.  Leave Arabia, or your pitiful legion will be destroyed!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 8**:
- "You take us for fools? Leave us, Roman!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 9**:
- "How much tribute will you require?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 3
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

**state 10**:
- "Most unlikely.  Try another approach.  Yes, this is most entertaining!"
  - Diplomatic [D] (code 3) "Your courage and wisdom are legendary." -> 5
  - Aggressive [A] (code 51) "Submit to the power of Rome!" -> 5
  - Offer Alliance [O] (code 50) "We offer an alliance." -> 5

**state 11**:
- "Ah, a diplomat! Most entertaining! Proceed, diplomat!"
  - Diplomatic [D] (code 3) "Your courage and wisdom are legendary." -> 5
  - Aggressive [A] (code 11) "Lay down your weapons." -> mood 0: 7, mood 1: 6 / weak 7, mood 2: 6 / weak 7
  - Offer Alliance [O] (code 50) "We offer an alliance." -> 5

## S = zone 19, Mesopotamia: Horaxus (LEADERS.CBM, TEXTS.LZW, 13 strings)

Scout: "Before your troops spreads the ancient valley of the Tigris and Euphrates, twin rivers that embrace the cradle of civilization."

**state 0**:
- "Leave us, barbarian, or defend yourselves."
- "We don't admit savages.  I suggest you withdraw."
- "Mesopotamia has no need of you."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "Many greetings.  Now, why should you interest us?"
  - Friendly [F] (code 8) "We should trust each other, my friend." -> 11
  - Diplomatic [D] (code 48) "Your courage and wisdom are legendary." -> mood 0: 12, mood 1: 12, mood 2: 4
  - Aggressive [A] (code 19) "Your choice is simple. Surrender with honor or die in disgrace." -> 7

**state 2**:
- "So let it be written.  Our mighty nations are now allies."
- "Your race is rustic, but vigorous.  You will make interesting allies."
- "Then we graciously consent to this alliance with the republic of Rome!"

**state 3**:
- "You demand booty? Have war instead!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "You honor us, Roman."
  - Diplomatic [D] (code 43) "Rome desires only peace between our great nations." -> 5
  - Aggressive [A] (code 11) "Lay down your weapons." -> 12
  - Offer Alliance [O] (code 49) "Then we offer an alliance, and the promise of protection." -> 6

**state 5**:
- "I see problems with this.  What course do you suggest?"
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 12
  - Aggressive [A] (code 11) "Lay down your weapons." -> 12
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> mood 0: 8 / weak 9, mood 1: 8 / weak 9, mood 2: 8 / weak 9

**state 6**:
- "You are insolent.  Take your barbaric rabble and go!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 7**:
- "Others have spoken thus; we remain, but they are dust.  Go or suffer their fate!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 8**:
- "What tribute would satisfy?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 2 (ALLIANCE, end)
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

**state 9**:
- "Your meagre force needs protection more than we."
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 10
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 10**:
- "Then we grant your wish for war!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "You are not so skilled a liar as you think."
  - Friendly [F] (code 20) "I am here to make peace." -> 12
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 12
  - Offer Alliance [O] (code 10) "We want you as allies." -> 12

**state 12**:
- "We are an ancient race, victors in many wars.  We are about to win one more!"
- "Primitive braggart! The heirs of Hammurabi shall slaughter you like sheep."
- "Fight, if you dare.  May the gods grant us strength to destroy you utterly."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

## T = zone 20, Armenia: Bertonus (LEADERT.CBM, TEXTT.LZW, 13 strings)

Scout: "Your scouts return from the rugged mountains of Armenia to report that the region is largely unpopulated.  But Armenian forces meet your troops soon after penetrating the border."

**state 0**:
- "Leave Armenia while you may!"
- "Be gone or feel the sting of our blades!"
- "Retreat or face certain defeat and death!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "Hail, commander.  You wish to have council?"
  - Friendly [F] (code 8) "We should trust each other, my friend." -> 11
  - Diplomatic [D] (code 48) "Your courage and wisdom are legendary." -> mood 0: 12, mood 1: 12, mood 2: 4
  - Aggressive [A] (code 19) "Your choice is simple. Surrender with honor or die in disgrace." -> 7

**state 2**:
- "So be it.  Make it known to all lesser races that we are allies."
- "You are a worthy ally.  Weaker nations shall tremble before us!"
- "We are happy to be your allies.  Might Romans be part Armenian?"

**state 3**:
- "You extort like a Cilician.  Prepare to die!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "Your words are gracious."
  - Diplomatic [D] (code 43) "Rome desires only peace between our great nations." -> 5
  - Aggressive [A] (code 11) "Lay down your weapons." -> 12
  - Offer Alliance [O] (code 49) "Then we offer an alliance, and the promise of protection." -> 6

**state 5**:
- "That may be hard to achieve.  What are your ideas?"
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 12
  - Aggressive [A] (code 11) "Lay down your weapons." -> 12
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> mood 0: 8 / weak 9, mood 1: 8 / weak 9, mood 2: 8 / weak 9

**state 6**:
- "Your words turn graceless as a Syrian's.  Leave us!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 7**:
- "You are worse than a Greek! We will seal your fate in blood!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 8**:
- "State the tribute you require."
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 3
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

**state 9**:
- "Or should we protect you? Parthians may be near."
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 10
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 10**:
- "If you crave battle, come have it!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "You lie better than even a Parthian."
  - Friendly [F] (code 20) "I am here to make peace." -> 12
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 12
  - Offer Alliance [O] (code 10) "We want you as allies." -> 12

**state 12**:
- "A man does not find his death, his death finds him.  Count yourself found!"
- "It is said a short life is a happy one.  We are glad to grant you such happiness!"
- "Your lies sicken us.  A curse on Romans everywhere!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

## U = zone 21, Sicilia: Hamilcar (LEADERU.CBM, TEXTU.LZW, 12 strings)

Scout: "Your forces arrive in Sicilia, and the scouts report that the island will not be easily conquered.  Its natural defenses include rugged countryside and the surrounding ocean."

**state 0**:
- "Scabrous one! We will drive you into the sea!"
- "Roman thief! You will sleep with the fishes!"
- "Dog! Ravens shall peck at your eyes this day!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "Speak, Roman, but know this audience will be brief."
  - Friendly [F] (code 2) "We come in friendship." -> 10
  - Diplomatic [D] (code 46) "Hear me, wise and honorable chieftain." -> 11
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 4

**state 2**:
- "Truly, your offer is one we cannot refuse.  We have many foes who will fear your might."
- "Agreed.  An alliance with Rome will protect us from our enemies."
- "We are willing to pay for your protection.  The alliance is made"

**state 3**:
- "And leave us naked? Unacceptable!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "Accursed liar! You shall have the battle you seem to crave."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 5**:
- "You sing sweetly, but your concert is over.  Be gone!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 6**:
- "So you are a warrior after all."
  - Diplomatic [D] (code 45) "We seek to negotiate with you." -> 8
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 4
  - Offer Alliance [O] (code 52) "We offer an alliance, and promise protection in exchange for tribute." -> 9

**state 7**:
- "You lead few men, yet make many demands.  Leave now, or die!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 8**:
- "We are not children.  Be gone!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 9**:
- "What would proper tribute be in the eyes of Rome?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 3
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

**state 10**:
- "That can never be.  But we are still listening."
  - Diplomatic [D] (code 3) "Your courage and wisdom are legendary." -> 5
  - Aggressive [A] (code 51) "Submit to the power of Rome!" -> 5
  - Offer Alliance [O] (code 50) "We offer an alliance." -> 5

**state 11**:
- "A diplomat in warrior's garb.  Amazing! Go on."
  - Diplomatic [D] (code 3) "Your courage and wisdom are legendary." -> 5
  - Aggressive [A] (code 11) "Lay down your weapons." -> mood 0: 7, mood 1: 6 / weak 7, mood 2: 6 / weak 7
  - Offer Alliance [O] (code 50) "We offer an alliance." -> 5

## V = zone 22, Sardinia: Darminus (LEADERV.CBM, TEXTV.LZW, 14 strings)

Scout: "Your fleet approaches the maritime nation of Sardinia, a center of trade in Mare Nostrum.  Your aides tell you the island has great strategic importance."

**state 0**:
- "On your guard, invader.  We defy you!"
- "We give you fair warning; go or fight!"
- "This is no mere threat: Go or die!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "Speak, Roman, but be brief."
  - Friendly [F] (code 2) "We come in friendship." -> mood 0: 12 / weak 13, mood 1: 12 / weak 13, mood 2: 12 / weak 13
  - Diplomatic [D] (code 3) "Your courage and wisdom are legendary." -> 9
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> 10

**state 2**:
- "Your terms are just.  We are allies."
- "We accept.  Let us be allies."
- "Acceptable.  Rome has an ally."

**state 3**:
- "Brigands, you would take everything!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "Does Rome distinguish between friends and slaves?"
  - Friendly [F] (code 8) "We should trust each other, my friend." -> 6
  - Diplomatic [D] (code 43) "Rome desires only peace between our great nations." -> 6
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> mood 0: 8, mood 1: 8, mood 2: 7

**state 5**:
- "Never.  The badge of your ally is a slave's yoke."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 6**:
- "Your words are luster without substance.  Be gone!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 7**:
- "What you say rings true.  What tribute would be adequate?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 3
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

**state 8**:
- "Rome's protective embrace is a stranglehold.  Leave us!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 9**:
- "Weasel words! Just say what you want."
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 6
  - Aggressive [A] (code 41) "Your choice is simple. Surrender with honor or die in disgrace." -> 11
  - Offer Alliance [O] (code 10) "We want you as allies." -> 5

**state 10**:
- "We spit at your bluster! Can you fight as well as you talk?"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "Do Romans think they alone know the arts of war?"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 12**:
- "Do Romans always lie so?"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 13**:
- "We honor your valor."
  - Friendly [F] (code 42) "Rome can be a powerful friend." -> 4
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 6
  - Offer Alliance [O] (code 10) "We want you as allies." -> 5

## W = zone 23, Britannia: Caswallon (LEADERW.CBM, TEXTW.LZW, 13 strings)

Scout: "You and your men cross the channel safely to Britannia, an island of green trees and fields, unpredictable weather and warlike barbarian tribes."

**state 0**:
- "Away with you, outlander, unless you crave death."
- "Leave, interloper! Quit our lands or fight."
- "Leave our island, wretch, or meet your fate."
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 11
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "Who violates our sacred soil?"
  - Offer Alliance [O] (code 17) "We come in peace and seek an ally." -> mood 0: 4, mood 1: 4, mood 2: 5
  - Diplomatic [D] (code 18) "Hear me, wise and honorable chieftain." -> mood 0: 7, mood 1: 7, mood 2: 6
  - Aggressive [A] (code 19) "Your choice is simple. Surrender with honor or die in disgrace." -> mood 0: 8, mood 1: 8, mood 2: 9

**state 2**:
- "I agree to an alliance.  But Roman rule will be hard to enforce on our island."
- "Then the alliance is made.  But I cannot say how my countrymen will react."
- "I'm convinced an alliance would be for the best.  But others may be more stubborn than I."

**state 3**:
- "Insolence! Let battle decide this!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "An ally? Go on, but know we are not fools."
  - Diplomatic [D] (code 23) "We will try to earn your trust." -> 11
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 5**:
- "Allied with you? Some say Rome's ally at dawn is her slave by dusk."
  - Friendly [F] (code 20) "I am here to make peace." -> 11
  - Diplomatic [D] (code 21) "Forgive our arrogance, honorable chieftain." -> 11
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> 10

**state 6**:
- "You, a diplomat? Most odd for your breed."
  - Friendly [F] (code 20) "I am here to make peace." -> 11
  - Diplomatic [D] (code 21) "Forgive our arrogance, honorable chieftain." -> 11
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> 10

**state 7**:
- "Your lies betray you.  Slay them, Britons!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 8**:
- "Speech is fleeting as dew.  A battle can endure longer than a man's promise."
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 11
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> mood 0: 11 / weak 3, mood 1: 11 / weak 3, mood 2: 11 / weak 3
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> 10

**state 9**:
- "Peace is a sweet fruit, but surrender is a pit to choke on.  Expect it not from us."
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 11
  - Aggressive [A] (code 24) "That is the choice we offer, and we will not make it again." -> 11
  - Offer Alliance [O] (code 25) "Then we offer an alliance, and the promise of protection." -> mood 0: 10, mood 1: 10, mood 2: 12 / weak 10

**state 10**:
- "Protection? From who? Protect YOURSELF, Roman!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "Enough! The council ends, the battle begins.  It is the only way we know."
- "Old lies freshly told.  Retreat or fight!"
- "We wait no longer.  Your silence insults us, your death is at hand!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 12**:
- "What tribute would suffice?"
  - Low [L] (code 14) "The lowest in the Republic." -> 2 (ALLIANCE, end)
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 3
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

## X = zone 24, Narbonensis: Galba (LEADERX.CBM, TEXTX.LZW, 13 strings)

Scout: "Along the coast of Mare Nostrum, ten days' march from Rome, you enter the region known as Narbonensis.  Your soldiers are tense with anticipation and eager for a fight."

**state 0**:
- "I NOT TALK MEN FROM SOUTH!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "YOU TRIBE WEAR CLOTHES.  NOT RIGHT!"
- "TRIBE FROM SOUTH WANT TALK?"
- "ATTACK HERE YOU TRIBE NOT SMART!"
  - Friendly [F] (code 2) "We come in friendship." -> 12
  - Diplomatic [D] (code 40) "Rome desires only peace between our great nations." -> mood 0: 3, mood 1: 4, mood 2: 4
  - Aggressive [A] (code 11) "Lay down your weapons." -> mood 0: 6 / weak 5, mood 1: 6 / weak 5, mood 2: 6 / weak 5

**state 2** (never reached):
- *(empty string)*

**state 3**:
- "I NOT KNOW WORDS THESE.  YOU LAUGH HERE US?"
  - Friendly [F] (code 37) "I am here to make peace." -> 10
  - Diplomatic [D] (code 38) "We will try to earn your trust." -> 7
  - Offer Alliance [O] (code 10) "We want you as allies." -> 11

**state 4**:
- "MAYBE YOU SPEAK TRUTH.  MAYBE NOT.  NOT MATTER US."
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 11
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 10
  - Offer Alliance [O] (code 6) "We offer an alliance." -> 8

**state 5**:
- "HA! YOU WEAK! US STRONG!"
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 9
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 6**:
- "NO! WEAPONS US KEEP."
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 10
  - Ultimatum [U] (code 1) "Surrender or die!" -> 9
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)

**state 7**:
- "FRIEND? YOU NOT FRIEND."
  - Friendly [F] (code 37) "I am here to make peace." -> 10
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 11
  - Offer Alliance [O] (code 10) "We want you as allies." -> 11

**state 8**:
- "UH-LIE-ANCE.  THIS WORD I NOT LIKE."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 9**:
- "NO! US FIGHT NOW!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 10**:
- "TALK OVER! KILL NOW!!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "NO! TALK DONE! KILL THEM!!!"
- "LIES! FIGHT THEM!!"
- "NO MORE TALK! ATTACK THEM!!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 12**:
- "YOU NOT FRIEND.  YOU HERE FIGHT!"
  - Friendly [F] (code 8) "We should trust each other, my friend." -> 7
  - Diplomatic [D] (code 38) "We will try to earn your trust." -> 7
  - Offer Alliance [O] (code 6) "We offer an alliance." -> 8

## Y = zone 25, Scythia: Chrysogones (LEADERY.CBM, TEXTY.LZW, 13 strings)

Scout: "Your troops march into Scythia.  In the wilds of Scythia are scattered tribes with warlike traditions and many courageous fighters.  The days ahead may be costly to Romans and Scythians alike."

**state 0**:
- "Leave us, invader.  Withdraw or prepare to fight."
- "Leave Scythia, foreign dog, or gird for battle!"
- "Go back from whence you came, or we attack!"
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 11
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "Who dares trespass in the domain of the Scythians?"
  - Offer Alliance [O] (code 17) "We come in peace and seek an ally." -> mood 0: 4, mood 1: 4, mood 2: 5
  - Diplomatic [D] (code 18) "Hear me, wise and honorable chieftain." -> mood 0: 7, mood 1: 7, mood 2: 6
  - Aggressive [A] (code 19) "Your choice is simple. Surrender with honor or die in disgrace." -> mood 0: 8, mood 1: 8, mood 2: 9

**state 2** (never reached):
- "I will agree to this alliance, but the Scythians? I cannot guarantee what they will do."
- "This alliance is acceptable to me, but the rest of Scythia may not agree."
- "Let us be allies, then, but I cannot speak for all my people."

**state 3**:
- "Unacceptable! Let the gods of war decide!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 4**:
- "An ally? Hear ye I will, trust ye I will not."
  - Diplomatic [D] (code 23) "We will try to earn your trust." -> 11
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 5**:
- "Seek ye an ally, then? Romans seeking allies are wolves posing as shepherds."
  - Friendly [F] (code 20) "I am here to make peace." -> 11
  - Diplomatic [D] (code 21) "Forgive our arrogance, honorable chieftain." -> 11
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> 10

**state 6**:
- "A diplomat would ye be, Roman trespasser?"
  - Friendly [F] (code 20) "I am here to make peace." -> 11
  - Diplomatic [D] (code 21) "Forgive our arrogance, honorable chieftain." -> 11
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> 10

**state 7**:
- "Deceive us, would ye? Call out the warriors!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 8**:
- "No meaning in these words.  Powerful are the Scythians, but honorable they are not."
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 11
  - Aggressive [A] (code 7) "Submit to the power of Rome!" -> mood 0: 11 / weak 3, mood 1: 11 / weak 3, mood 2: 11 / weak 3
  - Offer Alliance [O] (code 22) "Join us and the armies of Rome will protect you." -> 10

**state 9**:
- "Peace is best, but surrender? Fight ye, the Scythians will.  Surrender they will not."
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 11
  - Aggressive [A] (code 24) "That is the choice we offer, and we will not make it again." -> 11
  - Offer Alliance [O] (code 25) "Then we offer an alliance, and the promise of protection." -> mood 0: 10, mood 1: 10, mood 2: 12 / weak 10

**state 10**:
- "Protection? Scythians need not protection.  The Scythians are strong! Teach ye a lesson!"
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "Now my patience ends.  To battle must we go.  Never a choice for us, always battle."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 12**:
- "How much tribute must we pay?"
  - Low [L] (code 14) "The lowest in the Republic." -> 3
  - Medium [M] (code 15) "Our demands will not be unreasonable." -> 3
  - High [H] (code 16) "We require tribute befitting the grandeur of Rome." -> 3

## Z = zone 26, Sarmatia: Malodor the Impaler (LEADERZ.CBM, TEXTZ.LZW, 13 strings)

Scout: "The legions of Rome enter Sarmatia and the scouts return with descriptions of a vast region with many tribes and few leaders.  Still, a Sarmatian force soon gathers to meet your army."

**state 0**:
- "Sarmatia has no room for Romans.  You must leave."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 1**:
- "Rome sends a great warrior.  We have heard of you."
- "A great general arrives! Your fame precedes you."
- "Others have told us you would come.  They also told us what you would want."
  - Friendly [F] (code 2) "We come in friendship." -> 12
  - Diplomatic [D] (code 40) "Rome desires only peace between our great nations." -> mood 0: 3, mood 1: 4, mood 2: 4
  - Aggressive [A] (code 11) "Lay down your weapons." -> mood 0: 6 / weak 5, mood 1: 6 / weak 5, mood 2: 6 / weak 5

**state 2** (never reached):
- *(empty string)*

**state 3**:
- "Ah yes, the peace of slavery."
  - Friendly [F] (code 37) "I am here to make peace." -> 10
  - Diplomatic [D] (code 38) "We will try to earn your trust." -> 7
  - Offer Alliance [O] (code 10) "We want you as allies." -> 11

**state 4**:
- "There can be no peace between us."
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 11
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 10
  - Offer Alliance [O] (code 6) "We offer an alliance." -> 8

**state 5**:
- "We are not stupid.  Your weakness is apparent."
  - Ultimatum [U] (code 26) "Submit now, or be destroyed." -> 9
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 6**:
- "No, friend.  There will be no surrender."
  - Aggressive [A] (code 39) "Capitulate now, and there will be no bloodshed." -> 10
  - Ultimatum [U] (code 1) "Surrender or die!" -> 9
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)

**state 7**:
- "We trust no one.  It profits us not."
  - Friendly [F] (code 37) "I am here to make peace." -> 10
  - Diplomatic [D] (code 13) "We seek to negotiate with you." -> 11
  - Offer Alliance [O] (code 10) "We want you as allies." -> 11

**state 8**:
- "This can never be, for we will not serve you."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 9**:
- "You are a windbag.  You bore me."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 10**:
- "Lay down your weapons and we will spare you."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 11**:
- "Surrender and we can talk.  Or would you fight us?"
- "No.  Unless you leave Sarmatia, we must fight."
- "No, Roman.  You may surrender now."
  - Go to War [G] (code 4) "Then we are at war." END (battle B06E)
  - Withdraw [W] (code 5) "We leave you in peace." END (withdraw, no battle)

**state 12**:
- "Sarmatians make war, not peace."
  - Friendly [F] (code 8) "We should trust each other, my friend." -> 7
  - Diplomatic [D] (code 38) "We will try to earn your trust." -> 7
  - Offer Alliance [O] (code 6) "We offer an alliance." -> 8

