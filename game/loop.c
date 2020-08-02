#include "game.h"
#include "menu.h"
#include "play.h"
#include "loop.h"

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
            if (tasks[task][state] != NULL)
            {
                if (state == STATE_STOP)
                {
                    int result = tasks[task][state]();

                    if ((result > 0) && (result < TASKS))
                    {
                        task = result - 1;
                        break;
                    }
                }
                else if (state == STATE_DRAW)
                {
                    Uint32 timer = 0;
                    event_t event;

                    while (1)
                    {
                        if (SDL_TICKS_PASSED(timer, SDL_GetTicks()))
                        {
                            SDL_Delay(timer - SDL_GetTicks());
                        }
                        while (SDL_PollEvent(&event))
                        {
                            switch (event.type)
                            {
                                case SDL_KEYDOWN:
                                    switch (event.key.keysym.sym)
                                    {
                                        case SDLK_ESCAPE:
                                            game->events |= EVENT_QUIT;
                                            break;
                                    }
                                    break;
                                case SDL_QUIT:
                                    game->events |= EVENT_QUIT;
                                    break;
                            }
                        }

                        timer = SDL_GetTicks() + (1000 / FPS);
                        if (tasks[task][state]() != 0)
                        {
                            break;
                        }
                        game->events = 0;
                    }
                }
                else
                {
                    tasks[task][state]();
                }
            }
        }
    }
}

void game_loop(game_t *game)
{
    game_menu(game, tasks[LOAD]);
    game_play(game, tasks[PLAY]);
    loop(game);
}

