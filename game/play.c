#include "game.h"
#include "task.h"
#include "bitmap.h"

static game_t *game;

static bitmap_t *player[2];
static int dir[2];

static int counter;
static int ticks;

static const color_t colors[] =
{
    {89, 130, 210, 255},
};

enum
{
    BITMAP_BACKGROUND,
    BITMAP_PLAYER1,
    BITMAP_PLAYER2,
    BITMAPS
};

static bitmap_t *bitmaps[BITMAPS];

static void load_bitmaps(void)
{
    const char *resources[] =
    {
        [BITMAP_BACKGROUND] = "img/background.png",
        [BITMAP_PLAYER1] = "img/player1.png",
        [BITMAP_PLAYER2] = "img/player2.png",
    };

    for (size_t index = 0; index < BITMAPS; index++)
    {
        bitmaps[index] = bitmap_load(resources[index]);
    }
    player[0] = bitmaps[BITMAP_PLAYER1];
    player[1] = bitmaps[BITMAP_PLAYER2];
}

static void set_bitmaps_position(void)
{
    bitmap_set_position(
        bitmaps[BITMAP_BACKGROUND],
        0,
        0
    );
    bitmap_set_position(
        player[0],
        game->w / 2 - player[0]->h / 2,
        game->h / 8 - player[0]->h / 2
    );
    bitmap_set_position(
        player[1],
        game->w / 2 - player[1]->h / 2,
        game->h / 8 * 6 - player[1]->h / 2
    );
}

static void move_player(int index)
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
        player[index]->x -= velocity;
        if (player[index]->x < min[index].x)
        {
            player[index]->x = min[index].x;
        }
    }
    if (dir[index] & EVENT_KEY_RIGHT)
    {
        player[index]->x += velocity;
        if (player[index]->x + player[index]->w > max[index].x)
        {
            player[index]->x = max[index].x - player[index]->w;
        }
    }
    if (dir[index] & EVENT_KEY_UP)
    {
        player[index]->y -= velocity;
        if (player[index]->y < min[index].y)
        {
            player[index]->y = min[index].y;
        }
    }
    if (dir[index] & EVENT_KEY_DOWN)
    {
        player[index]->y += velocity;
        if (player[index]->y + player[index]->h > max[index].y)
        {
            player[index]->y = max[index].y - player[index]->h;
        }
    }
}

static void draw_players(void)
{
    render_draw_bitmap(player[0]);
    render_draw_bitmap(player[1]);
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
    set_bitmaps_position();
    render_set_color(&colors[0]);
    render_clear();
    render_draw_bitmap(bitmaps[BITMAP_BACKGROUND]);
    draw_players();
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
    move_player(0);
    move_player(1);
    render_clear();
    render_draw_bitmap(bitmaps[BITMAP_BACKGROUND]);
    draw_players();
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

