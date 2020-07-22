#include "types.h"
#include "load.h"

static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

typedef struct
{
    SDL_Texture *texture;
    int w, h;
} player_t;

static player_t player1;
static player_t player2;

static SDL_Rect rect = {0, 0, 0, 0};

static SDL_Color color0 = {89 - 32, 130 - 32, 210 - 32, 0};
static SDL_Color color1 = {89     , 130,      210,      0};
static SDL_Color color2 = {89 + 32, 130 + 32, 210 + 32, 0};
static SDL_Color color3 = {89 - 64, 130 - 64, 210 - 64, 0};

static const int size = 500;

static int xoffset = 0;
static int yoffset = 0;

static int stopped = 0;

static void load_player(player_t *player, const char *filename)
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
    load_player(&player1, "img/player1.png");
    load_player(&player2, "img/player2.png");
}

static void set_color(const SDL_Color *color)
{
    SDL_SetRenderDrawColor(
        renderer,
        color->r,
        color->g,
        color->b,
        color->a
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

static void draw_player(const player_t *player, int x, int y)
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

static void clear_rects(void)
{
    clear_rect(rect.x, size - rect.y - rect.h);
    if (rect.x < size / 3)
    {
        clear_rect(rect.x, rect.y);
        clear_rect(size - rect.x - rect.w, rect.y);
    }
    clear_rect(size - rect.x - rect.w, size - rect.y - rect.h);
}

static void resize_rects(void)
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

static void draw_rects(void)
{
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
}

static void draw_players(void)
{
    set_color(&color3);
    draw_rect(rect.x, size - rect.y - rect.h);
    draw_rect(size - rect.x - rect.w, size - rect.y - rect.h);
    draw_player(&player2, rect.x + 10, size - rect.y - rect.h - 10);
    draw_player(&player1, size - rect.x - rect.w + 10, size - rect.y - rect.h - 10);
    SDL_RenderPresent(renderer);
}

static int must_stop(void)
{
    return (rect.x >= size / 2) && (rect.w <= player1.w + 20);
}

static void stop_state(game_t *game)
{
    stopped = 1;
    SDL_Delay(5000);
    game->change_state();
}

static void *draw(void *game)
{
    if (stopped)
    {
        return NULL;
    }
    if (must_stop())
    {
        draw_players();
        stop_state(game);
    }
    else
    {
        clear_rects();
        resize_rects();
        draw_rects();
    }
    return NULL;
}

callback_t *game_load(game_t *game)
{
    renderer = game->renderer;
    texture = game->texture;
    xoffset = game->width / 2 - size / 2;
    yoffset = game->height / 2 - size / 2;
    load_resources();
    return draw;
}

