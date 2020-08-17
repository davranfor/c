#include "game.h"
#include "load.h"

static game_t *game;

static void init(void)
{
    if (TTF_Init() != 0)
    {
        SDL_Log("TTF_Init: %s", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    atexit(TTF_Quit);

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        SDL_Log("IMG_Init: %s", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    atexit(IMG_Quit);

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("SDL_Init: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    atexit(SDL_Quit);
}

static void load(void)
{
    game->font.renderer = TTF_OpenFont(game->font.name, game->font.size);
    if (game->font.renderer == NULL)
    {
        SDL_Log("TTF_OpenFont: %s", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    TTF_SizeUTF8(game->font.renderer, "g", &game->font.w, &game->font.h);

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
    if (game->font.renderer != NULL)
    {
        TTF_CloseFont(game->font.renderer);
    }
}

void game_load(game_t *this)
{
    game = this;
    init();
    atexit(clean);
    load();
}

