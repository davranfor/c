#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>

enum
{
    GAME_STATUS_NONE,
    GAME_STATUS_INTRO
};

typedef void *sdl_callback_t(void *);

typedef struct
{
    const char *name;
    int size;
    int w, h;
} sdl_font;

typedef struct
{
    const char *title;
    int width, height;
    SDL_Color color;
    sdl_font font;
    int status;
} sdl_game;

#endif /* GAME_H */

