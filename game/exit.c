#include "game.h"
#include "task.h"
#include "bitmap.h"
#include "sprite.h"

static game_t *game;

static const color_t colors[] =
{
    {89, 130, 210, 255},
};

enum
{
    BITMAP_BACKGROUND,
    BITMAPS
};

enum
{
    SPRITE_TEST,
    SPRITES
};

static bitmap_t *bitmaps[BITMAPS];
static sprite_t *sprites[SPRITES];

static void load_resources(void)
{
    bitmaps[BITMAP_BACKGROUND] = bitmap_load("img/background.png");
    sprites[SPRITE_TEST] = sprite_load("img/dead.png", 10, 1);
}

static void set_positions(void)
{
    bitmap_set_position(
        bitmaps[BITMAP_BACKGROUND],
        0,
        0
    );
    sprite_set_position(
        sprites[SPRITE_TEST],
        game->w / 2 - sprites[SPRITE_TEST]->w / 2,
        game->h / 2 - sprites[SPRITE_TEST]->h / 2
    );
}

static void init(void)
{
    load_resources();
    sprite_set_delay(sprites[SPRITE_TEST], 7);
}

static int start(int events)
{
    (void)events;
    set_positions();
    render_set_color(&colors[0]);
    render_clear();
    render_draw_bitmap(bitmaps[BITMAP_BACKGROUND]);
    render_present();
    return 0;
}

static int draw(int events)
{
    if (events & EVENT_QUIT)
    {
        return 1;
    }
    render_clear();
    render_draw_bitmap(bitmaps[BITMAP_BACKGROUND]);
    render_draw_sprite(sprites[SPRITE_TEST]);
    render_present();
    return 0;
}

static int stop(int events)
{
    (void)events;
    SDL_Log("Exiting");
    return 0;
}

void game_exit(game_t *this, callback_t *state[])
{
    game = this;
    init();
    state[STATE_START] = start;
    state[STATE_DRAW] = draw;
    state[STATE_STOP] = stop;
}

