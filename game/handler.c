#include "game.h"
#include "load.h"
#include "play.h"
#include "handler.h"

enum
{
    NONE,
    LOAD,
    PLAY,
    TASKS,
};

static callback_t *tasks[TASKS][STATES];

static void loop(game_t *game)
{
    for (int task = 1; task < TASKS; task++)
    {
        for (int state = 0; state < STATES; state++)
        {
            SDL_Event event;

            while (SDL_PollEvent(&event));
            if (tasks[task][state] != NULL)
            {
                if (state == STATE_STOP)
                {
                    int result = tasks[task][state](game);

                    if ((result > 0) && (result < TASKS))
                    {
                        task = result - 1;
                        break;
                    }
                }
                else if (state == STATE_DRAW)
                {
                    Uint32 timer = 0;

                    while (1)
                    {
                        if (SDL_TICKS_PASSED(timer, SDL_GetTicks()))
                        {
                            SDL_Delay(timer - SDL_GetTicks());
                        }
/*
                        SDL_Event event;

                        if (SDL_PollEvent(&event))
                        {
                            if (event.type == SDL_KEYDOWN)
                            {
                                // Cancel draw state 
                                if (event.key.keysym.sym == SDLK_ESCAPE)
                                {
                                    exit(EXIT_SUCCESS);
                                }
                            }
                            // Skip other cases (used with break instead of exit())
                            // while (SDL_PollEvent(&event));
                        }
*/
                        timer = SDL_GetTicks() + (1000 / FPS);
                        if (tasks[task][state](game) != 0)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    tasks[task][state](game);
                }
            }
        }
    }
}

void game_loop(game_t *game)
{
    game_load(game, tasks[LOAD]);
    game_play(game, tasks[PLAY]);
    loop(game);
}

