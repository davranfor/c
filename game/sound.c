#include "debug.h"
#include "mapper.h"
#include "sound.h"

sound_t *sound_create(const char *path)
{
    mixer_t *mixer = mapper_load_mixer(path);
    sound_t *sound = calloc(1, sizeof *sound);

    if (sound == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    sound->mixer = mixer;
    return sound;
}

void sound_destroy(sound_t *sound)
{
    free(sound);
}

void sound_set_volume(sound_t *sound, int volume)
{
     Mix_VolumeChunk(sound->mixer, volume);
}

void sound_play(sound_t *sound)
{
    Mix_PlayChannel(-1, sound->mixer, 0);
}

