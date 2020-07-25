#include "types.h"

void bitmap_load(bitmap_t *bitmap, SDL_Renderer *renderer, const char *filename)
{
    SDL_Surface *surface;

    surface = IMG_Load(filename);
    if (surface == NULL)
    {
        SDL_Log("IMG_Load: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    bitmap->texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (bitmap->texture == NULL)
    {
        SDL_Log("SDL_CreateTextureFromSurface: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    bitmap->w = surface->w;
    bitmap->h = surface->h;
    SDL_FreeSurface(surface);
}

