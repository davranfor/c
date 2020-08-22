#include "game.h"
#include "task.h"
#include "bitmap.h"

static game_t *game;

static int keys[2];
static int counter;
static int elapsed;

static rect_t viewport[3];

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

static void create_bitmaps(void)
{
    const char *resources[] =
    {
        [BITMAP_BACKGROUND] = "img/background.png",
        [BITMAP_PLAYER1] = "img/player1.png",
        [BITMAP_PLAYER2] = "img/player2.png",
    };

    for (size_t index = 0; index < BITMAPS; index++)
    {
        bitmaps[index] = bitmap_create(resources[index]);
    }
}

static void destroy_bitmaps(void)
{
    for (size_t index = 0; index < BITMAPS; index++)
    {
        bitmap_destroy(bitmaps[index]);
    }
}

static void set_positions(void)
{
    bitmap_set_position(
        BITMAP(BACKGROUND),
        0,
        0
    );
    bitmap_set_position(
        BITMAP(PLAYER1),
        viewport[1].w / 2 - BITMAP(PLAYER1)->w / 2,
        0
    );
    bitmap_set_position(
        BITMAP(PLAYER2),
        viewport[2].w / 2 - BITMAP(PLAYER2)->w / 2,
        0
    );
}

static void set_viewports(void)
{
    // Playable area
    viewport[0].x = 10;
    viewport[0].y = 10;
    viewport[0].w = game->w - 20;
    viewport[0].h = game->h - 20;

    // Player1 area
    viewport[1].x = 10;
    viewport[1].y = 10;
    viewport[1].w = game->w - 20;
    viewport[1].h = game->h / 2 - 10;

    // Player2 area
    viewport[2].x = 10;
    viewport[2].y = game->h / 2;
    viewport[2].w = game->w - 20;
    viewport[2].h = game->h / 2 - 10;
}

static void reset_keys(void)
{
    keys[0] = 0;
    keys[1] = 0;
    counter = 0;
    elapsed = 0;
}

static void set_keys(int value)
{
    if (counter++ == elapsed)
    {
        int r = rand();

        keys[0] = 0;
        keys[0] |= r & 1 ? EVENT_KEY_LEFT : EVENT_KEY_RIGHT;
        keys[0] |= r & 2 ? EVENT_KEY_UP : EVENT_KEY_DOWN;
        elapsed += r % 50 + 50;
    }
    keys[1] = value;
}

static void move_player(int index)
{
    bitmap_t *player = BITMAP(PLAYER1 + index);
    const rect_t *area = &viewport[index + 1];
    const int velocity = 5 * (index + 1);

    if (keys[index] & EVENT_KEY_LEFT)
    {
        player->x -= velocity;
        if (player->x < 0)
        {
            player->x = 0;
        }
    }
    if (keys[index] & EVENT_KEY_UP)
    {
        player->y -= velocity;
        if (player->y < 0)
        {
            player->y = 0;
        }
    }
    if (keys[index] & EVENT_KEY_RIGHT)
    {
        player->x += velocity;
        if (player->x + player->w > area->w)
        {
            player->x = area->w - player->w;
        }
    }
    if (keys[index] & EVENT_KEY_DOWN)
    {
        player->y += velocity;
        if (player->y + player->h > area->h)
        {
            player->y = area->h - player->h;
        }
    }
}

static void move_players(void)
{
    move_player(0);
    move_player(1);
}

static void draw_players(void)
{
    render_set_viewport(&viewport[1]);
    render_draw_bitmap(BITMAP(PLAYER1));
    render_set_viewport(&viewport[2]);
    render_draw_bitmap(BITMAP(PLAYER2));
    render_set_viewport(NULL);
}

static void init(void)
{
    atexit(destroy_bitmaps);
    create_bitmaps();
    set_viewports();
}

static int start(int events)
{
    (void)events;
    reset_keys();
    set_positions();
    render_set_color(&colors[0]);
    return 0;
}

static int draw(int events)
{
    if (events & EVENT_QUIT)
    {
        return 1;
    }
    set_keys(events & EVENT_KEYS);
    move_players();
    render_clear();
    render_draw_bitmap(BITMAP(BACKGROUND));
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

