#include "game.h"
#include "task.h"

static int stop(int events)
{
    (void)events;
    SDL_Log("exit game");
    return 0;
}

void game_exit(game_t *this, callback_t *state[])
{
    (void)this;
    state[STATE_STOP] = stop;
}

