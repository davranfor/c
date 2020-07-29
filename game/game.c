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
    if (surface == NULL)
    {
        SDL_Log("TTF_RenderUTF8_Blended: %s\n", TTF_GetError());
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

static int button_match(bitmap_t *button, int x, int y)
{
    if (
        ((x >= button->x) && (x <= button->x + button->w)) &&
        ((y >= button->y) && (y <= button->y + button->h))
    )
    {
        return 1;
    }
    return 0;
}

static void button_mod(int index, bitmap_t *button[], Uint8 mod)
{
    SDL_Rect area =
    {
        button[index]->x,
        button[index]->y,
        button[index]->w,
        button[index]->h
    };

    SDL_SetTextureColorMod(
        button[index]->texture,
        mod,
        mod,
        mod
    );
    SDL_RenderCopy(
        game->renderer,
        button[index]->texture,
        NULL,
        &area
    );
    SDL_RenderPresent(game->renderer);
}

static void button_down(int index, bitmap_t *button[], int clicked)
{
    if (index == clicked - 1)
    {
        return;
    }
    button_mod(index, button, 128);
    SDL_Log("Button down | index = %d\n", index);
}

static void button_up(bitmap_t *button[], int clicked)
{
    if (clicked != 0)
    {
        button_mod(clicked - 1, button, 255);
        SDL_Log("Button up\n");
    }
}

int button_clicked(bitmap_t *button[], int buttons)
{
    SDL_MouseButtonEvent *mouse;
    SDL_Event event;
    int clicked = 0;

    while (1)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_MOUSEBUTTONDOWN:
                    mouse = &event.button;
                    if (mouse->button == SDL_BUTTON_LEFT)
                    {
                        button_up(button, clicked);
                        for (int index = 0; index < buttons; index++)
                        {
                            if (button_match(button[index], mouse->x, mouse->y))
                            {
                                button_down(index, button, clicked);
                                clicked = index + 1;
                                break;
                            }
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    mouse = &event.button;
                    if ((mouse->button == SDL_BUTTON_LEFT) && (clicked != 0))
                    {
                        button_up(button, clicked);
                        if (button_match(button[clicked - 1], mouse->x, mouse->y))
                        {
                            return clicked;
                        }
                        clicked = 0;
                    }
                    break;
/*
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_KP_ENTER:
                        case SDLK_RETURN:
                            if (buttons > 0)
                            {
                                //button_down(0, button);
                            }
                            break;
                        case SDLK_ESCAPE:
                            if (buttons == 2)
                            {
                                //button_down(1, button);
                            }
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_KP_ENTER:
                        case SDLK_RETURN:
                            if (buttons > 0)
                            {
                                //button_up(0, button);
                            }
                            return 1;
                        case SDLK_ESCAPE:
                            if (buttons == 2)
                            {
                                //button_up(1, button);
                            }
                            return 2;
                    }
                    break;
                case SDL_QUIT:
                    return -1;
*/
            }
        }
    }
    return -1;
}

