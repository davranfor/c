#include "game.h"

static game_t *game;

void game_set(game_t *this)
{
    game = this;
}

game_t *game_get(void)
{
    return game;
}


SDL_Keycode game_keydown(void)
{
    SDL_Event event;

    while (1)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN)
            {
                return event.key.keysym.sym;
            }
        }
    }
    return 0;
}

void game_pause(void)
{
    SDL_Event event;

    while (1)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_MOUSEBUTTONDOWN:
                    return;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                            exit(EXIT_SUCCESS);
                            return;
                    }
                    return;
                case SDL_QUIT:
                    exit(EXIT_SUCCESS);
                    return;
            }
        }
    }
}

void game_quit(void)
{
    SDL_Event event;

    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
    return;
}

void bitmap_load(bitmap_t *bitmap, const char *filename)
{
    SDL_Surface *surface;

    surface = IMG_Load(filename);
    if (surface == NULL)
    {
        SDL_Log("IMG_Load: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    bitmap->texture = SDL_CreateTextureFromSurface(game->renderer, surface);
    if (bitmap->texture == NULL)
    {
        SDL_Log("SDL_CreateTextureFromSurface: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    bitmap->w = surface->w;
    bitmap->h = surface->h;
    SDL_FreeSurface(surface);
}

void bitmap_text(bitmap_t *bitmap, const char *text)
{
    SDL_Surface *surface;

    surface = TTF_RenderUTF8_Blended(
        game->font.renderer,
        text,
        (SDL_Color){0, 0, 0, 0}
    );
    bitmap->texture = SDL_CreateTextureFromSurface(game->renderer, surface);
    if (bitmap->texture == NULL)
    {
        SDL_Log("SDL_CreateTextureFromSurface: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    bitmap->w = surface->w;
    bitmap->h = surface->h;
    SDL_FreeSurface(surface);
}

