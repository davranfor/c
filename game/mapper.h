#ifndef MAPPER_H
#define MAPPER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

typedef struct
{
    SDL_Texture *texture;
    const char *path;
    int w, h;
} resource_t;

void mapper_init(SDL_Renderer *);
resource_t *mapper_load_resource(const char *);

#endif /* MAPPER_H */

