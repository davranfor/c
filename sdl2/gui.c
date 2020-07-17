#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include "gui.h"

static gui_app *app = NULL;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static TTF_Font *font = NULL;

static void sdl_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
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
    SDL_SetRenderDrawColor(
        renderer,
        app->window.color.r,
        app->window.color.g,
        app->window.color.b,
        app->window.color.a
    );
    SDL_RenderClear(renderer);
}

static void ttf_init(void)
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
}

static void init(void)
{
    sdl_init();
    ttf_init();
}

static void *draw_rect(void *data)
{
    SDL_Rect *rect = data;

    SDL_SetRenderDrawColor(
        renderer,
        app->window.color.r,
        app->window.color.g,
        app->window.color.b,
        app->window.color.a
    );
    if (rect->x + rect->w >= app->window.width)
    {
        int mid = app->window.width / 2;

        SDL_RenderFillRect(
            renderer,
            &(SDL_Rect){mid, mid, mid, mid}
        );
        rect->x = 0;
        rect->y = 0;
        rect->w = 0;
        rect->h = 0;
        return NULL;
    }
    SDL_RenderFillRect(renderer, rect);
    rect->x += 5;
    rect->y += 5;
    rect->w += 5;
    rect->h += 5;
    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 0);
    SDL_RenderFillRect(renderer, rect);
    SDL_SetRenderDrawColor(renderer, 128, 128, 128,0);
    SDL_RenderFillRect(
        renderer,
        &(SDL_Rect)
        {
            app->window.width - rect->x,
            app->window.height - rect->y,
            rect->w,
            rect->h
        }
    );
    SDL_RenderPresent(renderer);
    return NULL;
}

static Uint32 callback(Uint32 interval, void *param)
{
    SDL_Event event;
    SDL_UserEvent userevent;

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = (void *)(uintptr_t)draw_rect;
    userevent.data2 = param;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return interval;
}

static void loop(void)
{
    SDL_Rect rect = {0, 0, 0, 0};
    SDL_TimerID timer = SDL_AddTimer(50, callback, &rect);
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
                    callback_ptr *func = (callback_ptr *)(uintptr_t)event.user.data1;
                    func(event.user.data2);
                    break;
                }
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_F1:
                            draw_rect(&rect);
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

static void close(void)
{
    if (renderer != NULL)
    {
        SDL_DestroyRenderer(renderer);
    }
    if (window != NULL)
    {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();

    if (font != NULL)
    {
        TTF_CloseFont(font);
    }
    TTF_Quit();
}

void gui_main(gui_app *self)
{
    assert(self != NULL);
    atexit(close);
    app = self;
    init();
    loop();
}

