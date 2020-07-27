#include "game.h"
#include "load.h"

static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

static bitmap_t mountain;
static bitmap_t player1;
static bitmap_t player2;
static bitmap_t button1;
static bitmap_t button2;
static bitmap_t clouds;

static SDL_Rect rect;

static SDL_Color color0 = {89 - 32, 130 - 32, 210 - 32, 255};
static SDL_Color color1 = {89     , 130,      210,      255};

static int size = 503;

static int xoffset;
static int yoffset;

static void load_resources(void)
{
    bitmap_load(&mountain, "img/mountain.png");
    bitmap_load(&player1, "img/player1.png");
    bitmap_load(&player2, "img/player2.png");
    bitmap_load(&button1, "img/play.png");
    bitmap_load(&button2, "img/stop.png");
    bitmap_load(&clouds, "img/clouds.png");
}

static void init(game_t *game)
{
    renderer = game->renderer;
    texture = game->texture;
    load_resources();
    xoffset = game->width / 2 - size / 2;
    yoffset = game->height / 2 - size / 2;
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

static void fill_rect(int x, int y)
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

static void draw_frame(void)
{
    SDL_Color white = {255, 255, 255, 255};

    set_color(&white);

    SDL_Rect area =
    {
        xoffset - 2,
        yoffset - 2,
        size + 4,
        size + 4
    };

    SDL_RenderFillRect(
        renderer,
        &area
    );
}

static void draw_mountain(void)
{
    SDL_Rect area =
    {
        0,
        (size + yoffset * 2) - mountain.h,
        mountain.w,
        mountain.h
    };

    SDL_RenderCopy(
        renderer,
        mountain.texture,
        NULL,
        &area
    );
}

static void clear_clouds(void)
{
    SDL_Rect area =
    {
        size / 2 + xoffset - clouds.w / 2,
        size / 2 + yoffset - clouds.h / 2,
        clouds.w,
        clouds.h
    };

    SDL_RenderCopy(
        renderer,
        texture,
        &area,
        &area
    );
}

static void draw_clouds(void)
{
    SDL_Rect area =
    {
        size / 2 + xoffset - clouds.w / 2,
        size / 2 + yoffset - clouds.h / 2,
        clouds.w,
        clouds.h
    };

    SDL_RenderCopy(
        renderer,
        clouds.texture,
        NULL,
        &area
    );
}

static void draw_players(void)
{
    SDL_Rect area1 =
    {
        size / 4 + xoffset - player1.w / 2,
        size / 4 + yoffset - player1.h / 2,
        player1.w,
        player1.h
    };

    SDL_RenderCopy(
        renderer,
        player1.texture,
        NULL,
        &area1
    );

    SDL_Rect area2 =
    {
        size - size / 4 + xoffset - player2.w / 2,
        size / 4 + yoffset - player2.h / 2,
        player2.w,
        player2.h
    };

    SDL_RenderCopy(
        renderer,
        player2.texture,
        NULL,
        &area2
    );
}

static void draw_buttons(void)
{
    button1.x = size / 4 + xoffset - button1.w / 2;
    button1.y = size / 2 + size / 4 + yoffset - button1.h / 2;

    SDL_Rect area1 =
    {
        button1.x,
        button1.y,
        button1.w,
        button1.h
    };

    SDL_RenderCopy(
        renderer,
        button1.texture,
        NULL,
        &area1
    );

    button2.x = size - size / 4 + xoffset - button2.w / 2;
    button2.y = size / 2 + size / 4 + yoffset - button2.h / 2;

    SDL_Rect area2 =
    {
        button2.x,
        button2.y,
        button2.w,
        button2.h
    };

    SDL_RenderCopy(
        renderer,
        button2.texture,
        NULL,
        &area2
    );
}

static void clear_rects(void)
{
    clear_rect(rect.x, size - rect.y - rect.h);
    clear_rect(rect.x, rect.y);
    clear_rect(size - rect.x - rect.w, rect.y);
    clear_rect(size - rect.x - rect.w, size - rect.y - rect.h);
}

static void reset_rects(void)
{
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
}

static void move_rects(void)
{
    rect.x += 3;
    rect.y += 3;
    rect.w += 3;
    rect.h += 3;
}

static void fill_rects(void)
{
    set_color(&color0);
    fill_rect(rect.x, rect.y);
    fill_rect(size - rect.x - rect.w, size - rect.y - rect.h);
    set_color(&color1);
    fill_rect(rect.x, size - rect.y - rect.h);
    fill_rect(size - rect.x - rect.w, rect.y);
}

static void draw_rects(void)
{
    clear_rects();
    clear_clouds();
    move_rects();
    fill_rects();
    draw_clouds();
}

static void draw_menu(void)
{
    draw_mountain();
    draw_frame();
    fill_rects();
    draw_players();
    draw_buttons();
    draw_clouds();
}

static int must_stop(void)
{
    return rect.w >= size / 2;
}

static int start(game_t *game)
{
    (void)game;
    reset_rects();
    return 0;
}

static int draw(game_t *game)
{
    (void)game;
    if (must_stop())
    {

        return 1;
    }
    draw_rects();
    SDL_RenderPresent(renderer);
    return 0;
}

static int stop(game_t *game)
{
    (void)game;
    draw_menu();
    SDL_RenderPresent(renderer);
    if (game_keydown() != SDLK_ESCAPE)
    {
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        return 1;
    }
    return 0;
}

static void clean(void)
{
    if (mountain.texture != NULL)
    {
        SDL_DestroyTexture(mountain.texture);
    }
    if (player1.texture != NULL)
    {
        SDL_DestroyTexture(player1.texture);
    }
    if (player2.texture != NULL)
    {
        SDL_DestroyTexture(player2.texture);
    }
    if (clouds.texture != NULL)
    {
        SDL_DestroyTexture(clouds.texture);
    }
    if (button1.texture != NULL)
    {
        SDL_DestroyTexture(button1.texture);
    }
    if (button2.texture != NULL)
    {
        SDL_DestroyTexture(button2.texture);
    }
}

void game_load(game_t *game, callback_t *state[])
{
    init(game);
    atexit(clean);
    state[STATE_START] = start;
    state[STATE_DRAW] = draw;
    state[STATE_STOP] = stop;
}

