#ifndef SOUND_H
#define SOUND_H

#define SOUND(sound) sounds[SOUND_##sound]

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

typedef struct
{
    Mix_Chunk *mixer;
} sound_t;

sound_t *sound_create(const char *);
void sound_destroy(sound_t *);
void sound_set_volume(sound_t *, int);
void sound_play(sound_t *);

#endif /* SOUND_H */

