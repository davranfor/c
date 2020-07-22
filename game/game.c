#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "types.h"
#include "load.h"
#include "play.h"
#include "game.h"

/*
SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);
*/

enum
{
    GAME_STATE_NONE,
    GAME_STATE_LOAD,
    GAME_STATE_PLAY,
    GAME_STATES,
};

static callback_t *state[GAME_STATES];

static game_t *game = NULL;

static SDL_Window *window = NULL;

static TTF_Font *font = NULL;

static SDL_TimerID timer;

static void load_resources(void)
{
    SDL_Surface *surface = IMG_Load("img/background.png");

    if (surface == NULL)
    {
        fprintf(stderr, "IMG_Load: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    game->texture = SDL_CreateTextureFromSurface(game->renderer, surface);
    if (game->texture == NULL)
    {
        fprintf(stderr, "SDL_CreateTextureFromSurface: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_FreeSurface(surface);
}

static void init(void)
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

    game->renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (game->renderer == NULL)
    {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_RenderSetLogicalSize(game->renderer, game->width, game->height);
    load_resources();
    SDL_RenderCopy(game->renderer, game->texture, NULL, NULL);
}

static void load(void)
{
    state[GAME_STATE_LOAD] = game_load(game);
    state[GAME_STATE_PLAY] = game_play(game);
}

static Uint32 callback(Uint32 interval, void *param)
{
    SDL_Event event;
    SDL_UserEvent userevent;

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = param;
    //userevent.data2 = param;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return interval;
}

static void quit(void)
{
    SDL_Event event;

    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
}

static void change_state(void)
{
    if (game->state != GAME_STATE_NONE)
    {
        SDL_RemoveTimer(timer);
    }
    game->state++;
    if (game->state == GAME_STATES)
    {
        quit();
    }
    timer = SDL_AddTimer(30, callback, (void *)(uintptr_t)state[game->state]);
    if (timer == 0)
    {
        fprintf(stderr, "SDL_AddTimer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

static void loop(void)
{
    SDL_Event event;
    int quit = 0;

    change_state();
    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_USEREVENT:
                {
                    callback_t *func = (callback_t *)(uintptr_t)event.user.data1;

                    //func(event.user.data2);
                    func(game);
                    break;
                }
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_F1:
                            state[GAME_STATE_LOAD](game);
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
    SDL_RemoveTimer(timer);
}

static void close(void)
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
    SDL_Quit();
    IMG_Quit();

    if (font != NULL)
    {
        TTF_CloseFont(font);
    }
    TTF_Quit();
}

void game_run(game_t *self)
{
    assert(self != NULL);
    atexit(close);
    game = self;
    game->change_state = change_state;
    init();
    load();
    loop();
}

