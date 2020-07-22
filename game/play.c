#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "types.h"
#include "play.h"

static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

static int stopped = 0;

static int must_stop(void)
{
    return 1;
}

static void stop_state(game_t *game)
{
    stopped = 1;
    game->change_state();
}

static void *draw(void *game)
{
    if (stopped)
    {
        return NULL;
    }
    if (must_stop())
    {
        puts("playing");
        stop_state(game);
    }
    return NULL;
}

callback_t *game_play(game_t *game)
{
    (void)game;
    renderer = game->renderer;
    texture = game->texture;
    return draw;
}

