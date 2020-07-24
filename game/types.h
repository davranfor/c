#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

typedef struct
{
    const char *name;
    int size;
    int w, h;
} font_t;

typedef struct
{
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    const char *title;
    int width, height;
    font_t font;
} game_t;

typedef int (callback_t)(game_t *);

#endif /* TYPES_H */

