#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include "gui.h"

static gui *app = NULL;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static TTF_Font *font = NULL;

static void window_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
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
}

static void font_init(void)
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

static void renderer_init(void)
{
    renderer = SDL_CreateRenderer(window, -1,  0);
    SDL_SetRenderDrawColor(
        renderer,
        app->window.color.r,
        app->window.color.g,
        app->window.color.b,
        app->window.color.a
    );
    SDL_RenderClear(renderer);
}

static void init(void)
{
    window_init();
    font_init();
    renderer_init();
}

static void loop(void)
{
    SDL_Event event;
    int quit = 0;

    while (!quit)
    {
        SDL_WaitEvent(&event);
        switch (event.type)
        {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
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
        }
    }
}

static void close(void)
{
    if (font != NULL)
    {
        TTF_CloseFont(font);
        TTF_Quit();
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
}

void gui_main(gui *self)
{
    assert(self != NULL);
    atexit(close);
    app = self;
    init();
    loop();
}

