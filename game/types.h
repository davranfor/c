#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

typedef void *callback_t(void *);

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
    int state;
    void (*change_state)(void);
} game_t;

#endif /* TYPES_H */

