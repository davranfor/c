#include "game.h"
#include "load.h"

static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

static bitmap_t mountain;
static bitmap_t player1;
static bitmap_t player2;
static bitmap_t clouds;
//static bitmap_t title;
static bitmap_t play;

static SDL_Rect rect;

static SDL_Color color0 = {89 - 32, 130 - 32, 210 - 32, 255};
static SDL_Color color1 = {89     , 130,      210,      255};

static int size = 500;

static int xoffset;
static int yoffset;

static void load_resources(void)
{
    bitmap_load(&mountain, "img/mountain.png");
    bitmap_load(&player1, "img/player1.png");
    bitmap_load(&player2, "img/player2.png");
    bitmap_load(&clouds, "img/clouds.png");
    //bitmap_load(&title, "img/title.png");
    bitmap_load(&play, "img/play.png");
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

static void draw_player(const bitmap_t *player, int x, int y)
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
        (size / 2 + xoffset) - (clouds.w / 2),
        (size / 2 + yoffset) - (clouds.h / 2),
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
        (size / 2 + xoffset) - (clouds.w / 2),
        (size / 2 + yoffset) - (clouds.h / 2),
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
/*
static void draw_title(void)
{
    SDL_Rect area =
    {
        size / 2 + xoffset - title.w,
        size / 2 + yoffset,
        title.w,
        title.h
    };

    SDL_RenderCopy(
        renderer,
        title.texture,
        NULL,
        &area
    );
}
*/
static void draw_play(void)
{
    SDL_Rect area =
    {
        size - size / 4 + xoffset - play.w / 2,
        size / 2 + size / 4 + yoffset - play.h / 2,
        play.w,
        play.h
    };

    SDL_RenderCopy(
        renderer,
        play.texture,
        NULL,
        &area
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
    rect.x += 2;
    rect.y += 2;
    rect.w += 2;
    rect.h += 2;
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
    SDL_RenderPresent(renderer);
}

static void draw_players(void)
{
    draw_mountain();
    draw_frame();
    fill_rects();
    draw_player(&player1, /****/ size / 4 - player1.w / 2, size / 4 - player1.h / 2);
    draw_player(&player2, size - size / 4 - player2.w / 2, size / 4 - player1.h / 2);
    draw_play();
    //draw_title();
    draw_clouds();
    SDL_RenderPresent(renderer);
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
        draw_players();
        return 1;
    }
    else
    {
        draw_rects();
    }
    return 0;
}

static int stop(game_t *game)
{
    (void)game;
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
/*
    if (title.texture != NULL)
    {
        SDL_DestroyTexture(title.texture);
    }
*/
    if (play.texture != NULL)
    {
        SDL_DestroyTexture(play.texture);
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

