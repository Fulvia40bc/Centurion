#ifndef SND_AIL_H
#define SND_AIL_H

/* the game's sound on ail.c (snd_ail.c); main.c attaches it before title_boot. headless: no device, no synth */
void snd_ail_attach(int headless);

/* the music engine before snd_startup: 's' stereo FM (the default), 'g' General MIDI; swap mirrors the FM sides */
void snd_ail_set_music(char set, int swap);

#endif
