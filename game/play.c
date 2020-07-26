#include "game.h"
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

void game_play(game_t *game, callback_t *state[])
{
    init(game);
    state[STATE_START] = start;
    state[STATE_DRAW] = NULL;
    state[STATE_STOP] = stop;
}

