#include "game.h"
#include "task.h"
#include "bitmap.h"

static game_t *game;

static rect_t rect[2];
static int dir[2];

static int counter;
static int ticks;

static const color_t colors[] =
{
    {255,   0,   0, 255},
    {  0,   0, 255, 255},
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

static void reset_rects(void)
{
    rect[0].x = game->w / 2 - 20;
    rect[0].y = game->h / 8 - 20;
    rect[0].w = 40;
    rect[0].h = 40;

    rect[1].x = game->w / 2 - 20;
    rect[1].y = game->h / 8 * 7 - 20;
    rect[1].w = 40;
    rect[1].h = 40;
}

static void move_rect(int index)
{
    const int velocity = 5 * (index + 1);

    const point_t min[] =
    {
        {10, 10},
        {10, game->h / 2}
    };
    const point_t max[] =
    {
        {game->w - 10, game->h / 2},
        {game->w - 10, game->h - 10}
    };

    if (dir[index] & EVENT_KEY_LEFT)
    {
        rect[index].x -= velocity;
        if (rect[index].x < min[index].x)
        {
            rect[index].x = min[index].x;
        }
    }
    if (dir[index] & EVENT_KEY_RIGHT)
    {
        rect[index].x += velocity;
        if (rect[index].x + rect[index].w > max[index].x)
        {
            rect[index].x = max[index].x - rect[index].w;
        }
    }
    if (dir[index] & EVENT_KEY_UP)
    {
        rect[index].y -= velocity;
        if (rect[index].y < min[index].y)
        {
            rect[index].y = min[index].y;
        }
    }
    if (dir[index] & EVENT_KEY_DOWN)
    {
        rect[index].y += velocity;
        if (rect[index].y + rect[index].h > max[index].y)
        {
            rect[index].y = max[index].y - rect[index].h;
        }
    }
}

static void draw_rects(void)
{
    render_set_color(&colors[0]);
    render_fill_rect(&rect[0]);
    render_set_color(&colors[1]);
    render_fill_rect(&rect[1]);
}

static void randomize(int events)
{
    if (counter++ == ticks)
    {
        int r = rand();

        dir[0] = 0;
        dir[0] |= (r & 1) ? EVENT_KEY_LEFT : EVENT_KEY_RIGHT;
        dir[0] |= (r & 2) ? EVENT_KEY_UP : EVENT_KEY_DOWN;
        ticks += r % 50 + 50;
    }
    dir[1] = events;
}

static void init(void)
{
    load_bitmaps();
}

static int start(int events)
{
    (void)events;
    reset_rects();
    set_bitmaps_position();
    render_set_color(&colors[2]);
    render_clear();
    render_draw_bitmap(bitmaps[BITMAP_BACKGROUND]);
    draw_rects();
    render_present();
    return 0;
}

static int draw(int events)
{
    if (events & EVENT_QUIT)
    {
        return 1;
    }
    randomize(events);
    move_rect(0);
    move_rect(1);
    render_clear();
    render_draw_bitmap(bitmaps[BITMAP_BACKGROUND]);
    draw_rects();
    render_present();
    return 0;
}

static int stop(int events)
{
    (void)events;
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

