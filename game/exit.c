#include "game.h"
#include "task.h"
#include "bitmap.h"
#include "sprite.h"
#include "sound.h"

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
    SPRITE_DEAD,
    SPRITES
};

enum
{
    SOUND_BELL,
    SOUNDS
};

static const int resurrect[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1};

static bitmap_t *bitmaps[BITMAPS];
static sprite_t *sprites[SPRITES];
static sound_t *sounds[SOUNDS];

static void create_resources(void)
{
    BITMAP(BACKGROUND) = bitmap_create("img/background.png");
    SPRITE(DEAD) = sprite_create("img/dead.png", 10, 1);
    SOUND(BELL) = sound_create("snd/bell.wav");
}

static void destroy_resources(void)
{
    bitmap_destroy(BITMAP(BACKGROUND));
    sprite_destroy(SPRITE(DEAD));
    sound_destroy(SOUND(BELL));
}

static void set_delays(void)
{
    sprite_set_delay(SPRITE(DEAD), 5);
}

static void set_volumes(void)
{
    sound_set_volume(SOUND(BELL), 16);
}

static void set_positions(void)
{
    bitmap_set_position(
        BITMAP(BACKGROUND),
        0,
        0
    );
    sprite_set_position(
        SPRITE(DEAD),
        game->w / 2 - SPRITE(DEAD)->w / 2,
        game->h / 2 - SPRITE(DEAD)->h / 2
    );
}

static void init(void)
{
    atexit(destroy_resources);
    create_resources();
}

static int start(int events)
{
    (void)events;
    set_delays();
    set_volumes();
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
    render_clear();
    render_draw_bitmap(BITMAP(BACKGROUND));
    if (!sprite_is_animating(SPRITE(DEAD)))
    {
        if (sprite_get_sequence(SPRITE(DEAD)) == NULL)
        {
            sprite_play_sequence(SPRITE(DEAD), resurrect);
        }
        else
        {
            sound_play(SOUND(BELL));
            sprite_play_sequence(SPRITE(DEAD), NULL);
        }
    }
    render_draw_sprite(SPRITE(DEAD));
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

