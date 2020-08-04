#include "game.h"
#include "bitmap.h"
#include "menu.h"

static game_t *game;
static rect_t view;
static rect_t rect;

static const color_t colors[] =
{
    { 57,  98, 178, 255},
    { 89, 130, 210, 255},
    {255, 255, 255, 255}
};

enum
{
    BITMAP_BACKGROUND,
    BITMAP_GRADIENT,
    BITMAP_CLOUDS,
    BITMAP_PLAYER1,
    BITMAP_PLAYER2,
    BITMAP_PLAY,
    BITMAP_STOP,
    BITMAPS
};

static bitmap_t *bitmaps[BITMAPS];

static void load_bitmaps(void)
{
    const char *resources[] =
    {
        [BITMAP_BACKGROUND] = "img/background.png",
        [BITMAP_GRADIENT] = "img/gradient.png",
        [BITMAP_CLOUDS] = "img/clouds.png",
        [BITMAP_PLAYER1] = "img/player1.png",
        [BITMAP_PLAYER2] = "img/player2.png",
        [BITMAP_PLAY] = "img/play.png",
        [BITMAP_STOP] = "img/stop.png",
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
    bitmap_set_position(
        bitmaps[BITMAP_GRADIENT],
        0,
        0
    );
    bitmap_set_position(
        bitmaps[BITMAP_CLOUDS],
        view.x + view.w / 2 - bitmaps[BITMAP_CLOUDS]->w / 2,
        view.y + view.h / 2 - bitmaps[BITMAP_CLOUDS]->h / 2
    );
    bitmap_set_position(
        bitmaps[BITMAP_PLAYER1],
        view.x + view.w / 4 - bitmaps[BITMAP_PLAYER1]->w / 2,
        view.y + view.h / 4 - bitmaps[BITMAP_PLAYER1]->h / 2
    );
    bitmap_set_position(
        bitmaps[BITMAP_PLAYER2],
        view.x + view.w - view.w / 4 - bitmaps[BITMAP_PLAYER2]->w / 2,
        view.y + view.h / 4 - bitmaps[BITMAP_PLAYER2]->h / 2
    );
    bitmap_set_position(
        bitmaps[BITMAP_PLAY],
        view.x + view.w / 4 - bitmaps[BITMAP_PLAY]->w / 2,
        view.y + view.h / 2 + view.h / 4 - bitmaps[BITMAP_PLAY]->h / 2
    );
    bitmap_set_position(
        bitmaps[BITMAP_STOP],
        view.x + view.w - view.w / 4 - bitmaps[BITMAP_STOP]->w / 2,
        view.y + view.h / 2 + view.h / 4 - bitmaps[BITMAP_STOP]->h / 2
    );
}

static void set_view_coords(void)
{
    view.w = 503;
    view.h = 503;
    view.x = game->w / 2 - view.w / 2;
    view.y = game->h / 2 - view.h / 2;
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

static void draw_rects(void)
{
    render_set_color(&colors[0]);
    render_fill_area(
        view.x + rect.x,
        view.y + rect.y,
        rect.w,
        rect.h
    );
    render_fill_area(
        view.x + view.w - rect.x - rect.w,
        view.y + view.h - rect.y - rect.h,
        rect.w,
        rect.h
    );
    render_set_color(&colors[1]);
    render_fill_area(
        view.x + rect.x,
        view.y + view.h - rect.y - rect.h,
        rect.w,
        rect.h
    );
    render_fill_area(
        view.x + view.w - rect.x - rect.w,
        view.y + rect.y,
        rect.w,
        rect.h
    );
}

static void draw_frame(void)
{
    render_set_color(&colors[2]);
    render_fill_area(view.x - 2, view.y - 2, view.w + 4, view.h + 4);
}

static void draw_players(void)
{
    render_draw_bitmap(bitmaps[BITMAP_PLAYER1]);
    render_draw_bitmap(bitmaps[BITMAP_PLAYER2]);
}

static void draw_buttons(void)
{
    render_draw_bitmap(bitmaps[BITMAP_PLAY]);
    render_draw_bitmap(bitmaps[BITMAP_STOP]);
}

static void draw_animation(void)
{
    render_draw_bitmap(bitmaps[BITMAP_GRADIENT]);
    move_rects();
    draw_rects();
    render_draw_bitmap(bitmaps[BITMAP_CLOUDS]);
}

static void draw_menu(void)
{
    render_draw_bitmap(bitmaps[BITMAP_BACKGROUND]);
    draw_frame();
    draw_rects();
    draw_players();
    draw_buttons();
    render_draw_bitmap(bitmaps[BITMAP_CLOUDS]);
}

static int must_stop(void)
{
    return rect.w >= view.w / 2;
}

static void init(void)
{
    load_bitmaps();
    set_view_coords();
}

static int start(int events)
{
    (void)events;
    reset_rects();
    set_bitmaps_position();
    render_set_color(&colors[1]);
    render_clear();
    render_draw_bitmap(bitmaps[BITMAP_GRADIENT]);
    render_present();
    /* A delay in order to synchronize with the window manager */
    game_delay(100);
    return 0;
}

static int draw(int events)
{
    (void)events;
    if (must_stop())
    {
        return 1;
    }
    render_clear();
    draw_animation();
    render_present();
    return 0;
}

static int stop(int events)
{
    (void)events;
    render_clear();
    draw_menu();
    render_present();

    bitmap_t *buttons[] =
    {
        bitmaps[BITMAP_PLAY],
        bitmaps[BITMAP_STOP]
    };

    int clicked = button_clicked(buttons, 2);

    if (clicked != 1)
    {
        exit(EXIT_SUCCESS);
        //return TASK_MENU;
    }
    return 0;
}

void game_menu(game_t *this, callback_t *state[])
{
    game = this;
    init();
    state[STATE_START] = start;
    state[STATE_DRAW] = draw;
    state[STATE_STOP] = stop;
}

