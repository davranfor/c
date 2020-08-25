#ifndef SOUND_H
#define SOUND_H

#define SOUND(sound) sounds[SOUND_##sound]

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

typedef struct
{
    Mix_Chunk *mixer;
    int channel;
    int volume;
} sound_t;

sound_t *sound_create(const char *);
void sound_destroy(sound_t *);
void sound_set_volume(sound_t *, int);
int sound_get_volume(const sound_t *);
void sound_play(sound_t *);
void sound_loop(sound_t *);
void sound_pause(const sound_t *);
void sound_resume(const sound_t *);
void sound_stop(const sound_t *);
int sound_is_playing(const sound_t *);
int sound_is_paused(const sound_t *);
void sound_pause_all(void);
void sound_resume_all(void);
void sound_stop_all(void);
int sound_channels_playing(void);
int sound_channels_paused(void);

#endif /* SOUND_H */

