#include "mapper.h"
#include "bitmap.h"
#include "sprite.h"
#include "game.h"

static SDL_Renderer *renderer;

void game_init(game_t *game)
{
    renderer = game->renderer;
    mapper_init(renderer);
    bitmap_init(renderer);
    sprite_init(renderer);
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
        SDL_Delay(1);
    }
    return key;
}

SDL_Keycode game_keyup(void)
{
    SDL_Keycode key = 0;
    SDL_Event event;
    int done = 0;

    while (!done)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYUP)
            {
                key = event.key.keysym.sym;
                done = 1;
            }
        }
        SDL_Delay(1);
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
        SDL_Delay(1);
    }
}

void render_set_viewport(const SDL_Rect *rect)
{
    SDL_RenderSetViewport(renderer, rect);
}

void render_set_color(const SDL_Color *color)
{
    SDL_SetRenderDrawColor(
        renderer,
        color->r,
        color->g,
        color->b,
        color->a
    );
}

void render_fill_rect(const SDL_Rect *rect)
{
    SDL_RenderFillRect(renderer, rect);
}

void render_draw_rect(const SDL_Rect *rect)
{
    SDL_RenderDrawRect(renderer, rect);
}

void render_fill_area(int x, int y, int w, int h)
{
    SDL_Rect area = {x, y, w, h};

    SDL_RenderFillRect(renderer, &area);
}

void render_draw_area(int x, int y, int w, int h)
{
    SDL_Rect area = {x, y, w, h};

    SDL_RenderDrawRect(renderer, &area);
}

void render_clear(void)
{
    SDL_RenderClear(renderer);
}

void render_present(void)
{
    SDL_RenderPresent(renderer);
}

SDL_Texture *texture_create(const char *path)
{
    SDL_Texture *texture;

    texture = IMG_LoadTexture(renderer, path);
    if (texture == NULL)
    {
        SDL_Log("IMG_LoadTexture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    return texture;
}

