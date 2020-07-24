#include "types.h"
#include "load.h"
#include "play.h"
#include "task.h"

#define FPS 30

enum
{
    NONE,
    LOAD,
    PLAY,
    TASKS,
};

enum
{
    START,
    DRAW,
    STOP,
    SUBTASKS
};

static callback_t *task[TASKS][SUBTASKS];

static void init(game_t *game)
{
    game_load(game, task[LOAD]);
    game_play(game, task[PLAY]);
}

static void quit(void)
{
    SDL_Event event;

    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
    return;
}

static void exec(game_t *game)
{
    for (int i = 1; i < TASKS; i++)
    {
        for (int j = 0; j < SUBTASKS; j++)
        {
            if (task[i][j] != NULL)
            {
                if (j == STOP)
                {
                    int res = task[i][j](game);

                    if ((res > 0) && (res < TASKS))
                    {
                        i = res - 1;
                        break;
                    }
                }
                else if (j == DRAW)
                {
                    Uint32 timer = 0;

                    while (1)
                    {
                        if (SDL_TICKS_PASSED(timer, SDL_GetTicks()))
                        {
                            SDL_Delay(timer - SDL_GetTicks());
                        }
                        timer = SDL_GetTicks() + (1000 / FPS);

                        SDL_Event event;

                        if (SDL_PollEvent(&event))
                        {
                            if (event.type == SDL_KEYDOWN)
                            {
                                // Cancel draw state 
                                if (event.key.keysym.sym == SDLK_ESCAPE)
                                {
                                    exit(0);
                                }
                            }
                            // Skip other cases (used with break instead of exit()
                            // while (SDL_PollEvent(&event));
                        }
                        if (task[i][j](game) != 0)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    task[i][j](game);
                }
            }
        }
    }
}

void game_exec(game_t *game)
{
    init(game);
    exec(game);
    quit();
}

