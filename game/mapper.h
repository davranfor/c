#ifndef MAPPER_H
#define MAPPER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

typedef struct
{
    SDL_Texture *texture;
    int w, h;
} image_t;

typedef Mix_Chunk mixer_t;

void mapper_init(SDL_Renderer *);
image_t *mapper_load_image(const char *);
mixer_t *mapper_load_mixer(const char *);

#endif /* MAPPER_H */

