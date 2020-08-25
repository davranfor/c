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
    sound->volume = 64;
    return sound;
}

void sound_destroy(sound_t *sound)
{
    free(sound);
}

void sound_set_volume(sound_t *sound, int volume)
{
    sound->volume = volume;
}

int sound_get_volume(const sound_t *sound)
{
    return sound->volume;
}

void sound_play(sound_t *sound)
{
    Mix_VolumeChunk(sound->mixer, sound->volume);
    sound->channel = Mix_PlayChannel(-1, sound->mixer, 0);
}

void sound_loop(sound_t *sound)
{
    Mix_VolumeChunk(sound->mixer, sound->volume);
    sound->channel = Mix_PlayChannel(-1, sound->mixer, -1);
}

void sound_pause(const sound_t *sound)
{
    if (Mix_GetChunk(sound->channel) == sound->mixer)
    {
        Mix_Pause(sound->channel);
    }
}

void sound_resume(const sound_t *sound)
{
    if (Mix_GetChunk(sound->channel) == sound->mixer)
    {
        Mix_Resume(sound->channel);
    }
}

void sound_stop(const sound_t *sound)
{
    if (Mix_GetChunk(sound->channel) == sound->mixer)
    {
        Mix_HaltChannel(sound->channel);
    }
}

int sound_is_playing(const sound_t *sound)
{
    if (Mix_GetChunk(sound->channel) == sound->mixer)
    {
        return Mix_Playing(sound->channel);
    }
    return 0;
}

int sound_is_paused(const sound_t *sound)
{
    if (Mix_GetChunk(sound->channel) == sound->mixer)
    {
        return Mix_Paused(sound->channel);
    }
    return 0;
}

void sound_pause_all(void)
{
    Mix_Pause(-1);
}

void sound_resume_all(void)
{
    Mix_Resume(-1);
}

void sound_stop_all(void)
{
    Mix_HaltChannel(-1);
}

int sound_channels_playing(void)
{
    return Mix_Playing(-1);
}

int sound_channels_paused(void)
{
    return Mix_Paused(-1);
}

