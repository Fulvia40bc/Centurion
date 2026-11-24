#ifndef SND_AIL_H
#define SND_AIL_H

/* the game's sound on ail.c (snd_ail.c); main.c attaches it before title_boot. headless: no device, no synth, no
   settings written */
void snd_ail_attach(int headless);

/* the music choice before snd_startup: first the settings file advciv.ini next to advciv.exe, then the command line:
   set 's' stereo FM (the default), 'f' FM, 'g' General MIDI, 0 keeps it; swap 1 mirrors the FM sides, 0 not, -1
   keeps it */
void snd_ail_load_settings(void);
void snd_ail_set_music(char set, int swap);

#endif
