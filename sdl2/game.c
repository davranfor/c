#include <SDL2/SDL.h>
#include "game.h"
#include "intro.h"

static sdl_game *game = NULL;

static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

static SDL_Color color0 = { 64,  64,  64, 0};
static SDL_Color color1 = {128, 128, 128, 0};
static SDL_Color color2 = {160, 160, 160, 0};

static void randomize_colors(void)
{
    color1.r = (Uint8)(rand() % 64 + 128);
    color1.g = (Uint8)(rand() % 64 + 128);
    color1.b = (Uint8)(rand() % 64 + 128);

    color0.r = (Uint8)(color1.r - 32);
    color0.g = (Uint8)(color1.g - 32);
    color0.b = (Uint8)(color1.b - 32);

    color2.r = (Uint8)(color1.r + 32);
    color2.g = (Uint8)(color1.g + 32);
    color2.b = (Uint8)(color1.b + 32);
}

static void adjust_rect(SDL_Rect *rect, int max)
{
    rect->x += 5;
    rect->y += 5;
    if (rect->x + rect->w < max)
    {
        rect->w += 5;
        rect->h += 5;
    }
    else
    {
        rect->w = max - rect->x;
        rect->h = max - rect->y;
    }
}

void intro_init(sdl_game *intro_game, SDL_Renderer *intro_renderer, SDL_texture *intro_texture)
{
    game = intro_game;
    renderer = intro_renderer;
    texture = intro_texture;
}

static void *sdl_draw(void *data)
{
    const int max = game->width;

    SDL_Rect *rect = data;

    if (rect->x >= max)
    {
        randomize_colors();
        rect->x = 0;
        rect->y = 0;
        rect->w = 0;
        rect->h = 0;
        return NULL;
    }

    SDL_RenderCopy(
        renderer,
        texture,
        rect,
        rect
    );
    SDL_RenderCopy(
        renderer,
        texture,
        &(SDL_Rect) {rect->x, max - rect->y - rect->h, rect->w, rect->h},
        &(SDL_Rect) {rect->x, max - rect->y - rect->h, rect->w, rect->h}
    );
    SDL_RenderCopy(
        renderer,
        texture,
        &(SDL_Rect) {max - rect->x - rect->w, rect->y, rect->w, rect->h},
        &(SDL_Rect) {max - rect->x - rect->w, rect->y, rect->w, rect->h}
    );
    SDL_RenderCopy(
        renderer,
        texture,
        &(SDL_Rect) {max - rect->x - rect->w, max - rect->y - rect->h, rect->w, rect->h},
        &(SDL_Rect) {max - rect->x - rect->w, max - rect->y - rect->h, rect->w, rect->h}
    );

    adjust_rect(rect, max);

    SDL_SetRenderDrawColor(
        renderer,
        color0.r,
        color0.g,
        color0.b,
        color0.a
    );
    SDL_RenderFillRect(
        renderer,
        rect
    );
    SDL_RenderFillRect(
        renderer,
        &(SDL_Rect) {max - rect->x - rect->w, max - rect->y - rect->h, rect->w, rect->h}
    );
    SDL_SetRenderDrawColor(
        renderer,
        color1.r,
        color1.g,
        color1.b,
        color1.a
    );
    SDL_RenderFillRect(
        renderer,
        &(SDL_Rect) {rect->x, max - rect->y - rect->h, rect->w, rect->h}
    );
    if (rect->x < max / 3)
    {
        SDL_SetRenderDrawColor(
            renderer,
            color2.r,
            color2.g,
            color2.b,
            color2.a
        );
        SDL_RenderFillRect(
            renderer,
            &(SDL_Rect) {max / 2 - rect->w / 2, max / 2 - rect->y / 2, rect->w, rect->h}
        );
        SDL_SetRenderDrawColor(
            renderer,
            color1.r,
            color1.g,
            color1.b,
            color1.a
        );
    }
    SDL_RenderFillRect(
        renderer,
        &(SDL_Rect) {max - rect->x - rect->w, rect->y, rect->w, rect->h}
    );

    SDL_RenderPresent(renderer);
    return NULL;
}

