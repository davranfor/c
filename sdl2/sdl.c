#include <assert.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "game.h"
#include "intro.h"
#include "sdl.h"

/*
SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);
*/

static sdl_game *game = NULL;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

static TTF_Font *font = NULL;

static void load_resources(void)
{
    SDL_Surface *surface = IMG_Load("background.png");

    if (surface == NULL)
    {
        fprintf(stderr, "IMG_Load: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL)
    {
        fprintf(stderr, "SDL_CreateTextureFromSurface: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_FreeSurface(surface);
}

static void sdl_init(void)
{
    if (TTF_Init() != 0)
    {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    font = TTF_OpenFont(game->font.name, game->font.size);
    if (font == NULL)
    {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    TTF_SizeUTF8(font, "g", &game->font.w, &game->font.h);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        fprintf(stderr, "IMG_Init: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow(
        game->title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        game->width,
        game->height,
        SDL_WINDOW_SHOWN
    );
    if (window == NULL)
    {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_RenderSetLogicalSize(renderer, game->width, game->height);
    SDL_SetRenderDrawColor(
        renderer,
        game->color.r,
        game->color.g,
        game->color.b,
        game->color.a
    );
    SDL_RenderClear(renderer);
    load_resources();
    SDL_RenderCopy(renderer, texture, NULL, NULL);
}

static Uint32 sdl_callback(Uint32 interval, void *param)
{
    SDL_Event event;
    SDL_UserEvent userevent;

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = (void *)(uintptr_t)intro_draw;
    userevent.data2 = param;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return interval;
}

static void sdl_loop(void)
{
    SDL_TimerID timer = SDL_AddTimer(30, sdl_callback, NULL);

    if (timer == 0)
    {
        fprintf(stderr, "SDL_AddTimer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Event event;
    int quit = 0;

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_USEREVENT:
                {
                    sdl_callback_t *func = (sdl_callback_t *)(uintptr_t)event.user.data1;
                    func(event.user.data2);
                    break;
                }
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_F1:
                            intro_draw(NULL);
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
                            SDL_RenderPresent(renderer);
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
    SDL_RemoveTimer(timer);
}

static void sdl_close(void)
{
    if (texture != NULL)
    {
        SDL_DestroyTexture(texture);
    }
    if (renderer != NULL)
    {
        SDL_DestroyRenderer(renderer);
    }
    if (window != NULL)
    {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
    IMG_Quit();

    if (font != NULL)
    {
        TTF_CloseFont(font);
    }
    TTF_Quit();
}

static void game_init(void)
{
    intro_init(game, renderer, texture);
}

void sdl_main(sdl_game *self)
{
    assert(self != NULL);
    atexit(sdl_close);
    game = self;
    sdl_init();
    game_init();
    sdl_loop();
}

