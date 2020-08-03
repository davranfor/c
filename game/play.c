#include "game.h"
#include "bitmap.h"
#include "play.h"

static game_t *game;
static rect_t rect;

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

static void init(void)
{
    load_bitmaps();
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

static void clear_rect(void)
{
    render_clear_area(
        bitmaps[BITMAP_BACKGROUND],
        rect.x,
        rect.y,
        rect.w,
        rect.h
    );
}

static void fill_rect(void)
{
    render_fill_area(
        rect.x,
        rect.y,
        rect.w,
        rect.h
    );
}

static int direction_x(int events)
{
    int mask = EVENT_KEY_LEFT | EVENT_KEY_RIGHT;

    if ((events & mask) == 0)
    {
        return 0;
    }
    if ((events & mask) == mask)
    {
        return 0;
    }
    return events & mask;
}

static int direction_y(int events)
{
    int mask = EVENT_KEY_UP | EVENT_KEY_DOWN;

    if ((events & mask) == 0)
    {
        return 0;
    }
    if ((events & mask) == mask)
    {
        return 0;
    }
    return events & mask;
}

static int start(int events)
{
    (void)events;
    reset_rect();
    set_bitmaps_position();
    render_draw(bitmaps[BITMAP_BACKGROUND]);
    fill_rect();
    render_present();
    return 0;
}

static int draw(int events)
{
    if (events == EVENT_QUIT)
    {
        return 1;
    }

    int move_x = direction_x(events);
    int move_y = direction_y(events);

    if (move_x || move_y)
    {
        clear_rect();
        move_rect(move_x);
        move_rect(move_y);
        fill_rect();
        render_present();
    }
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

