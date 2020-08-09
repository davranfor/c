#include "game.h"
#include "task.h"
#include "loop.h"

static callback_t *tasks[TASKS][STATES];

static void loop(void)
{
    for (int task = 1; task < TASKS; task++)
    {
        if (tasks[task][STATE_START] != NULL)
        {
            tasks[task][STATE_START](0);
        }
        if (tasks[task][STATE_DRAW] != NULL)
        {
            Uint32 timer = 0;
            SDL_Event event;
            int events = 0;

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
                            if (!event.key.repeat)
                            {
                                if (event.key.keysym.sym == SDLK_UP)
                                {
                                    events |= EVENT_KEY_UP;
                                }
                                if (event.key.keysym.sym == SDLK_DOWN)
                                {
                                    events |= EVENT_KEY_DOWN;
                                }
                                if (event.key.keysym.sym == SDLK_LEFT)
                                {
                                    events |= EVENT_KEY_LEFT;
                                }
                                if (event.key.keysym.sym == SDLK_RIGHT)
                                {
                                    events |= EVENT_KEY_RIGHT;
                                }
                                if (event.key.keysym.sym == SDLK_ESCAPE)
                                {
                                    event.type = SDL_QUIT;
                                    SDL_PushEvent(&event);
                                }
                            }
                            break;
                        case SDL_KEYUP:
                            if (event.key.keysym.sym == SDLK_UP)
                            {
                                events &= ~EVENT_KEY_UP;
                            }
                            if (event.key.keysym.sym == SDLK_DOWN)
                            {
                                events &= ~EVENT_KEY_DOWN;
                            }
                            if (event.key.keysym.sym == SDLK_LEFT)
                            {
                                events &= ~EVENT_KEY_LEFT;
                            }
                            if (event.key.keysym.sym == SDLK_RIGHT)
                            {
                                events &= ~EVENT_KEY_RIGHT;
                            }
                            break;
                        case SDL_QUIT:
                            events |= EVENT_QUIT;
                            break;
                    }
                }
                timer = SDL_GetTicks() + (1000 / FPS);
                if (tasks[task][STATE_DRAW](events) != 0)
                {
                    break;
                }
            }
        }
        if (tasks[task][STATE_STOP] != NULL)
        {
            int result = tasks[task][STATE_STOP](0);

            if ((result > 0) && (result < TASKS))
            {
                task = result - 1;
            }
        }
    }
}

void game_loop(game_t *game)
{
    game_menu(game, tasks[TASK_MENU]);
    game_play(game, tasks[TASK_PLAY]);
    game_exit(game, tasks[TASK_EXIT]);
    loop();
}

