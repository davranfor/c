#include "game.h"
#include "bitmap.h"
#include "play.h"

static game_t *game;

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

static int start(void)
{
    set_bitmaps_position();
    render_draw(bitmaps[BITMAP_BACKGROUND]);
    render_present();
    SDL_Log("start playing");
    return 0;
}

static int draw(void)
{
    if (game->events & EVENT_QUIT)
    {
        return 1;
    }
    return 0;
}

static int stop(void)
{
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

