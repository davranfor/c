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
    SDL_Keycode key = 0;
    SDL_Event event;
    int done = 0;

    while (!done)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN)
            {
                key = event.key.keysym.sym;
                done = 1;
            }
        }
    }
    return key;
}

void game_pause(void)
{
    SDL_Event event;
    int done = 0;

    while (!done)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_MOUSEBUTTONDOWN:
                case SDL_KEYDOWN:
                    done = 1;
                    break;
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

void render_clear(void)
{
    SDL_RenderClear(game->renderer);
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

