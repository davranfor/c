#include "game.h"
#include "bitmap.h"
#include "play.h"

static game_t *game;
static rect_t rect;

static const color_t colors[] =
{
    { 89, 130, 210, 255},
};

enum
{
    BITMAP_BACKGROUND,
    BITMAPS
};

static bitmap_t *bitmaps[BITMAPS];

static void load_bitmaps(void)
{
    const char *resources[] =
    {
        [BITMAP_BACKGROUND] = "img/background.png",
    };

    for (size_t index = 0; index < BITMAPS; index++)
    {
        bitmaps[index] = bitmap_load(resources[index]);
    }
}

static void set_bitmaps_position(void)
{
    bitmap_set_position(
        bitmaps[BITMAP_BACKGROUND],
        0,
        0
    );
}

static void reset_rect(void)
{
    rect.x = game->w / 2 - 20;
    rect.y = game->h / 2 - 20;
    rect.w = 40;
    rect.h = 40;
}

static void move_rect(int direction)
{
    int velocity = 10;

    switch (direction)
    {
        case EVENT_KEY_UP:
            rect.y -= velocity;
            if (rect.y < 0)
            {
                rect.y = 0;
            }
            break;
        case EVENT_KEY_DOWN:
            rect.y += velocity;
            if (rect.y + rect.h > game->h)
            {
                rect.y = game->h - rect.h;
            }
            break;
        case EVENT_KEY_LEFT:
            rect.x -= velocity;
            if (rect.x < 0)
            {
                rect.x = 0;
            }
            break;
        case EVENT_KEY_RIGHT:
            rect.x += velocity;
            if (rect.x + rect.w > game->w)
            {
                rect.x = game->w - rect.w;
            }
            break;
    }
}

static void init(void)
{
    load_bitmaps();
}

static int start(int events)
{
    (void)events;
    reset_rect();
    set_bitmaps_position();
    render_set_color(&colors[0]);
    render_clear();
    render_draw_bitmap(bitmaps[BITMAP_BACKGROUND]);
    render_fill_rect(&rect);
    render_present();
    return 0;
}

static int draw(int events)
{
    if (events == EVENT_QUIT)
    {
        return 1;
    }
    render_clear();
    render_draw_bitmap(bitmaps[BITMAP_BACKGROUND]);
    move_rect(events & (EVENT_KEY_LEFT | EVENT_KEY_RIGHT));
    move_rect(events & (EVENT_KEY_UP | EVENT_KEY_DOWN));
    render_fill_rect(&rect);
    render_present();
    return 0;
}

static int stop(int events)
{
    (void)events;
    SDL_Log("stop playing");
    return 0;
}

void game_play(game_t *this, callback_t *state[])
{
    game = this;
    init();
    state[STATE_START] = start;
    state[STATE_DRAW] = draw;
    state[STATE_STOP] = stop;
}

