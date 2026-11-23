#ifndef SND_AIL_H
#define SND_AIL_H

/* the game's sound on ail.c (snd_ail.c); main.c attaches it before title_boot. headless: no device, no SoundFont */
void snd_ail_attach(int headless);

#endif
