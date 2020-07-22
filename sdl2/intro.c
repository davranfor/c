#include <SDL2/SDL_image.h>
#include "game.h"
#include "intro.h"

static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

typedef struct
{
    SDL_Texture *texture;
    int w, h;
} sdl_player;

static sdl_player player1;
static sdl_player player2;

static SDL_Rect rect = {0, 0, 0, 0};

static SDL_Color color0 = { 89 - 32, 130 - 32, 210 - 32, 0};
static SDL_Color color1 = { 89     , 130,      210,      0};
static SDL_Color color2 = { 89 + 32, 130 + 32, 210 + 32, 0};
static SDL_Color color3 = { 89 - 64, 130 - 64, 210 - 64, 0};

static const int size = 500;

static int xoffset = 0;
static int yoffset = 0;

static int drawable = 1;

static void load_player(sdl_player *player, const char *filename)
{
    SDL_Surface *surface;

    surface = IMG_Load(filename);
    if (surface == NULL)
    {
        fprintf(stderr, "IMG_Load: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    player->texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (player->texture == NULL)
    {
        fprintf(stderr, "SDL_CreateTextureFromSurface: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    player->w = surface->w;
    player->h = surface->h;
    SDL_FreeSurface(surface);
}

static void load_resources(void)
{
    load_player(&player1, "player1.png");
    load_player(&player2, "player2.png");
}

void intro_init(sdl_game *game, SDL_Renderer *p_renderer, SDL_Texture *p_texture)
{
    renderer = p_renderer;
    texture = p_texture;
    xoffset = game->width / 2 - size / 2;
    yoffset = game->height / 2 - size / 2;
    load_resources();
}

static void set_color(SDL_Color *color)
{
    SDL_SetRenderDrawColor(
        renderer,
        color->r,
        color->g,
        color->b,
        color->a
    );
}

static void draw_rect(int x, int y)
{
    SDL_Rect area =
    {
        x + xoffset,
        y + yoffset,
        rect.w,
        rect.h
    };

    SDL_RenderFillRect(
        renderer,
        &area
    );
}

static void clear_rect(int x, int y)
{
    SDL_Rect area =
    {
        x + xoffset,
        y + yoffset,
        rect.w,
        rect.h
    };

    SDL_RenderCopy(
        renderer,
        texture,
        &area,
        &area
    );
}

static void draw_player(sdl_player *player, int x, int y)
{
    SDL_Rect area =
    {
        x + xoffset,
        y + yoffset,
        player->w,
        player->h
    };

    SDL_RenderCopy(
        renderer,
        player->texture,
        NULL,
        &area
    );
}

static void adjust_rect(void)
{
    rect.x += 5;
    rect.y += 5;
    if (rect.x + rect.w < size)
    {
        rect.w += 5;
        rect.h += 5;
    }
    else
    {
        rect.w = size - rect.x;
        rect.h = size - rect.y;
    }
}

void *intro_draw(void *data)
{
    (void)data;

    if (drawable == 0)
    {
        return NULL;
    }

    if ((rect.x >= size / 2) && (rect.w <= 160))
    {
        set_color(&color3);
        draw_rect(rect.x, size - rect.y - rect.h);
        draw_rect(size - rect.x - rect.w, size - rect.y - rect.h);
        draw_player(&player1, rect.x + 5, size - rect.y - rect.h - 10);
        draw_player(&player2, size - rect.x - rect.w + 5, size - rect.y - rect.h - 10);
        SDL_RenderPresent(renderer);
        drawable = 0;
        return NULL;
    }
/*
    if (rect.x >= size)
    {
        rect.x = 0;
        rect.y = 0;
        rect.w = 0;
        rect.h = 0;
        return NULL;
    }
*/
    clear_rect(rect.x, size - rect.y - rect.h);
    if (rect.x < size / 3)
    {
        clear_rect(rect.x, rect.y);
        clear_rect(size - rect.x - rect.w, rect.y);
    }
    clear_rect(size - rect.x - rect.w, size - rect.y - rect.h);
    adjust_rect();
    set_color(&color0);
    if (rect.x < size / 3)
    {
        draw_rect(rect.x, rect.y);
    }
    draw_rect(size - rect.x - rect.w, size - rect.y - rect.h);
    set_color(&color1);
    draw_rect(rect.x, size - rect.y - rect.h);
    if (rect.x < size / 3)
    {
        set_color(&color2);
        draw_rect(size / 2 - rect.w / 2, size / 2 - rect.y / 2);
        set_color(&color1);
        draw_rect(size - rect.x - rect.w, rect.y);
    }
    SDL_RenderPresent(renderer);
    return NULL;
}

