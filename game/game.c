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

static void button_down(int index, bitmap_t *button[], bitmap_t *pressed[])
{
    if ((pressed == NULL) || (pressed[index] == NULL))
    {
        return;
    }

    SDL_Rect area =
    {
        button[index]->x,
        button[index]->y,
        button[index]->w,
        button[index]->h
    };

    SDL_RenderCopy(
        game->renderer,
        pressed[index]->texture,
        NULL,
        &area
    );
    SDL_RenderPresent(game->renderer);
    SDL_Log("Button down | index = %d\n", index);
}

static void button_up(int index, bitmap_t *button[], bitmap_t *pressed[])
{
    if ((pressed == NULL) || (pressed[index] == NULL))
    {
        return;
    }

    SDL_Rect area =
    {
        button[index]->x,
        button[index]->y,
        button[index]->w,
        button[index]->h
    };

    SDL_RenderCopy(
        game->renderer,
        button[index]->texture,
        NULL,
        &area
    );
    SDL_RenderPresent(game->renderer);
    SDL_Log("Button up   | index = %d\n", index);
}

int button_clicked(bitmap_t *button[], bitmap_t *pressed[], int buttons)
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
                        for (int i = 0; i < buttons; i++)
                        {
                            if (button_match(button[i], mouse->x, mouse->y))
                            {
                                button_down(i, button, pressed);
                                clicked = i + 1;
                                break;
                            }
                            clicked = 0;
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    mouse = &event.button;
                    if ((mouse->button == SDL_BUTTON_LEFT) && (clicked != 0))
                    {
                        button_up(clicked - 1, button, pressed);
                        if (button_match(button[clicked - 1], mouse->x, mouse->y))
                        {
                            return clicked;
                        }
                        clicked = 0;
                    }
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_KP_ENTER:
                        case SDLK_RETURN:
                            return 0;
                        case SDLK_ESCAPE:
                            return -1;
                    }
                    break;
                case SDL_QUIT:
                    return -1;
            }
        }
    }
    return -1;
}

