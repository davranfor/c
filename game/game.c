#include <assert.h>
#include "types.h"
#include "task.h"
#include "game.h"

static game_t *game = NULL;

static SDL_Window *window = NULL;

static TTF_Font *font = NULL;

static void init(void)
{
    if (TTF_Init() != 0)
    {
        SDL_Log("TTF_Init: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    atexit(TTF_Quit);

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        SDL_Log("IMG_Init: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    atexit(IMG_Quit);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        SDL_Log("SDL_Init: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    atexit(SDL_Quit);
}

static void load_resources(void)
{
    game->texture = IMG_LoadTexture(game->renderer, "img/background.png");
    if (game->texture == NULL)
    {
        SDL_Log("IMG_LoadTexture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_RenderCopy(game->renderer, game->texture, NULL, NULL);
}

static void load(void)
{
    font = TTF_OpenFont(game->font.name, game->font.size);
    if (font == NULL)
    {
        SDL_Log("TTF_OpenFont: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    TTF_SizeUTF8(font, "g", &game->font.w, &game->font.h);

    window = SDL_CreateWindow(
        game->title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        game->width,
        game->height,
        SDL_WINDOW_SHOWN
    );
    if (window == NULL)
    {
        SDL_Log("SDL_CreateWindow: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    game->renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (game->renderer == NULL)
    {
        SDL_Log("SDL_CreateRenderer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    load_resources();
}

static void exec(void)
{
    game_exec(game);
}

static void loop(void)
{
    SDL_Event event;
    int quit = 0;

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_F1:
                            break;
                        case SDLK_ESCAPE:
                            quit = 1;
                            break;
                    }
                    break;
                case SDL_WINDOWEVENT:
                    switch (event.window.event)
                    {
                        case SDL_WINDOWEVENT_EXPOSED:
                            SDL_RenderPresent(game->renderer);
                            break;
                    }
                    break;
                case SDL_QUIT:
                    quit = 1;
                    break;
                default:
                    break;
            }
        }
    }
}

static void clean(void)
{
    if (game->texture != NULL)
    {
        SDL_DestroyTexture(game->texture);
    }
    if (game->renderer != NULL)
    {
        SDL_DestroyRenderer(game->renderer);
    }
    if (window != NULL)
    {
        SDL_DestroyWindow(window);
    }
    if (font != NULL)
    {
        TTF_CloseFont(font);
    }
}

void game_run(game_t *this)
{
    game = this;
    init();
    atexit(clean);
    load();
    exec();
    loop();
}

