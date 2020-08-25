#include "game.h"
#include "load.h"

static game_t *game;

static void init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        SDL_Log("SDL_Init: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    atexit(SDL_Quit);

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        SDL_Log("IMG_Init: %s", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    atexit(IMG_Quit);

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
    {
        SDL_Log("Mix_OpenAudio: %s", Mix_GetError());
        exit(EXIT_FAILURE);
    }
    atexit(Mix_CloseAudio);
}

static void load(void)
{
    game->window = SDL_CreateWindow(
        game->title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        game->w,
        game->h,
        SDL_WINDOW_SHOWN
    );
    if (game->window == NULL)
    {
        SDL_Log("SDL_CreateWindow: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    game->renderer = SDL_CreateRenderer(
        game->window,
        -1,
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED
    );
    if (game->renderer == NULL)
    {
        SDL_Log("SDL_CreateRenderer: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

static void clean(void)
{
    if (game->renderer != NULL)
    {
        SDL_DestroyRenderer(game->renderer);
    }
    if (game->window != NULL)
    {
        SDL_DestroyWindow(game->window);
    }
}

void game_load(game_t *this)
{
    game = this;
    init();
    atexit(clean);
    load();
}

