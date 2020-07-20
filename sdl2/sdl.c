#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include "sdl.h"

/*
SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);
*/

typedef void *sdl_callback_t(void *);

static sdl_app *app = NULL;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

static TTF_Font *font = NULL;

static void sdl_init(void)
{
    if (TTF_Init() != 0)
    {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    font = TTF_OpenFont(app->font.name, app->font.size);
    if (font == NULL)
    {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    TTF_SizeUTF8(font, "g", &app->font.width, &app->font.height);

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
        app->window.title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        app->window.width,
        app->window.height,
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
    SDL_RenderSetLogicalSize(renderer, app->window.width, app->window.height);
    SDL_SetRenderDrawColor(
        renderer,
        app->window.color.r,
        app->window.color.g,
        app->window.color.b,
        app->window.color.a
    );
    SDL_RenderClear(renderer);

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
    SDL_RenderCopy(renderer, texture, NULL, NULL);
}

static SDL_Color cl0 = { 64,  64,  64, 255};
static SDL_Color cl1 = {128, 128, 128, 255};
static SDL_Color cl2 = {160, 160, 160, 255};

static void randomize_colors(void)
{
    cl1.r = (Uint8)(rand() % 64 + 128);
    cl1.g = (Uint8)(rand() % 64 + 128);
    cl1.b = (Uint8)(rand() % 64 + 128);

    cl0.r = (Uint8)(cl1.r - 32);
    cl0.g = (Uint8)(cl1.g - 32);
    cl0.b = (Uint8)(cl1.b - 32);

    cl2.r = (Uint8)(cl1.r + 32);
    cl2.g = (Uint8)(cl1.g + 32);
    cl2.b = (Uint8)(cl1.b + 32);
}

static void adjust_rect(SDL_Rect *rect, int max)
{
    rect->x += 5;
    rect->y += 5;
    if (rect->x + rect->w < max)
    {
        rect->w += 5;
        rect->h += 5;
    }
    else
    {
        rect->w = max - rect->x;
        rect->h = max - rect->y;
    }
}

static void *sdl_draw(void *data)
{
    const int max = app->window.width;

    SDL_Rect *rect = data;

    if (rect->x >= max)
    {
        randomize_colors();
        rect->x = 0;
        rect->y = 0;
        rect->w = 0;
        rect->h = 0;
        return NULL;
    }

    SDL_RenderCopy(
        renderer,
        texture,
        rect,
        rect
    );
    SDL_RenderCopy(
        renderer,
        texture,
        &(SDL_Rect) {rect->x, max - rect->y - rect->h, rect->w, rect->h},
        &(SDL_Rect) {rect->x, max - rect->y - rect->h, rect->w, rect->h}
    );
    SDL_RenderCopy(
        renderer,
        texture,
        &(SDL_Rect) {max - rect->x - rect->w, rect->y, rect->w, rect->h},
        &(SDL_Rect) {max - rect->x - rect->w, rect->y, rect->w, rect->h}
    );
    SDL_RenderCopy(
        renderer,
        texture,
        &(SDL_Rect) {max - rect->x - rect->w, max - rect->y - rect->h, rect->w, rect->h},
        &(SDL_Rect) {max - rect->x - rect->w, max - rect->y - rect->h, rect->w, rect->h}
    );

    adjust_rect(rect, max);

    SDL_SetRenderDrawColor(renderer, cl0.r, cl0.g, cl0.b, cl0.a);
    SDL_RenderFillRect(
        renderer,
        rect
    );
    SDL_RenderFillRect(
        renderer,
        &(SDL_Rect) {max - rect->x - rect->w, max - rect->y - rect->h, rect->w, rect->h}
    );
    SDL_SetRenderDrawColor(renderer, cl1.r, cl1.g, cl1.b, cl1.a);
    SDL_RenderFillRect(
        renderer,
        &(SDL_Rect) {rect->x, max - rect->y - rect->h, rect->w, rect->h}
    );
    if (rect->x < max / 3)
    {
        SDL_SetRenderDrawColor(renderer, cl2.r, cl2.g, cl2.b, cl2.a);
        SDL_RenderFillRect(
            renderer,
            &(SDL_Rect) {max / 2 - rect->w / 2, max / 2 - rect->y / 2, rect->w, rect->h}
        );
        SDL_SetRenderDrawColor(renderer, cl1.r, cl1.g, cl1.b, cl1.a);
    }
    SDL_RenderFillRect(
        renderer,
        &(SDL_Rect) {max - rect->x - rect->w, rect->y, rect->w, rect->h}
    );

    SDL_RenderPresent(renderer);
    return NULL;
}

static Uint32 sdl_callback(Uint32 interval, void *param)
{
    SDL_Event event;
    SDL_UserEvent userevent;

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = (void *)(uintptr_t)sdl_draw;
    userevent.data2 = param;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return interval;
}

static void sdl_loop(void)
{
    SDL_Rect rect = {0, 0, 0, 0};
    SDL_TimerID timer = SDL_AddTimer(30, sdl_callback, &rect);

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
                            sdl_draw(&rect);
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

void sdl_main(sdl_app *self)
{
    assert(self != NULL);
    atexit(sdl_close);
    app = self;
    sdl_init();
    sdl_loop();
}

