#include "bitmap.h"
#include "game.h"

static game_t *game;

void game_init(game_t *this)
{
    game = this;
    bitmap_init(game->renderer);
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

void render_set_color(const SDL_Color *color)
{
    SDL_SetRenderDrawColor(
        game->renderer,
        color->r,
        color->g,
        color->b,
        color->a
    );
}

void render_fill_rect(const SDL_Rect *rect)
{
    SDL_RenderFillRect(game->renderer, rect);
}

void render_draw_rect(const SDL_Rect *rect)
{
    SDL_RenderDrawRect(game->renderer, rect);
}

void render_fill_area(int x, int y, int w, int h)
{
    SDL_Rect area = {x, y, w, h};

    SDL_RenderFillRect(game->renderer, &area);
}

void render_draw_area(int x, int y, int w, int h)
{
    SDL_Rect area = {x, y, w, h};

    SDL_RenderDrawRect(game->renderer, &area);
}

void render_present(void)
{
    SDL_RenderPresent(game->renderer);
}

SDL_Texture *texture_create(const char *path)
{
    SDL_Texture *texture;

    texture = IMG_LoadTexture(game->renderer, path);
    if (texture == NULL)
    {
        SDL_Log("IMG_LoadTexture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_RenderCopy(game->renderer, texture, NULL, NULL);
    return texture;
}

