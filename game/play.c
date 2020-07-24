#include "types.h"
#include "play.h"

static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

static void init(game_t *game)
{
    renderer = game->renderer;
    texture = game->texture;
}

static int start(game_t *game)
{
    (void)game;
    SDL_Log("start playing");
    return 0;
}

static int stop(game_t *game)
{
    (void)game;
    SDL_Log("stop playing");
    return 0;
}

void game_play(game_t *game, callback_t *task[])
{
    init(game);
    task[0] = start;
    task[1] = NULL;
    task[2] = stop;
}

