#include "debug.h"
#include "mapper.h"
#include "sprite.h"

static SDL_Renderer *renderer;

void sprite_init(SDL_Renderer *this)
{
    renderer = this;
}

sprite_t *sprite_create(const char *path, int cols, int rows)
{
    assert((cols > 0) && (rows > 0));

    image_t *image = mapper_load_image(path);
    sprite_t *sprite = SDL_calloc(1, sizeof *sprite);

    if (sprite == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    sprite->texture = image->texture;
    sprite->w = image->w / cols;
    sprite->h = image->h / rows;
    sprite->frames = cols * rows;
    sprite->rows = rows;
    sprite->cols = cols;
    sprite->delay = 10;
    return sprite;
}

void sprite_destroy(sprite_t *sprite)
{
    free(sprite);
}

void sprite_set_position(sprite_t *sprite, int x, int y)
{
    sprite->x = x;
    sprite->y = y;
}

void sprite_get_position(const sprite_t *sprite, int *x, int *y)
{
    *x = sprite->x;
    *y = sprite->y;
}

void sprite_set_frames(sprite_t *sprite, int frames)
{
    assert((frames > 0) && (frames <= sprite->cols * sprite->rows));
    sprite->frames = frames;
}

int sprite_get_frames(const sprite_t *sprite)
{
    return sprite->frames;
}

void sprite_set_frame(sprite_t *sprite, int frame)
{
    assert(frame < sprite->frames);
    sprite->frame = frame;
}

int sprite_get_frame(const sprite_t *sprite)
{
    return sprite->frame;
}

int sprite_get_index(const sprite_t *sprite)
{
    if (sprite->sequence == NULL)
    {
        return sprite->frame;
    }
    else
    {
        assert(sprite->sequence[sprite->frame] < sprite->frames);
        return sprite->sequence[sprite->frame];
    }
}

void sprite_set_delay(sprite_t *sprite, int delay)
{
    assert(delay > 0);
    sprite->delay = delay;
}

int sprite_get_delay(const sprite_t *sprite)
{
    return sprite->delay;
}

void sprite_set_sequence(sprite_t *sprite, const int *sequence)
{
    assert((sequence == NULL) || (*sequence >= 0));
    if (sprite->sequence != sequence)
    {
        sprite->sequence = sequence;
        sprite->frame = 0;
        sprite->ticks = 0;
    }
}

const int *sprite_get_sequence(const sprite_t *sprite)
{
    return sprite->sequence;
}

static void sprite_set_state(sprite_t *sprite, int state)
{
    if (sprite->state == SPRITE_STOPPED)
    {
        sprite->frame = 0;
        sprite->ticks = 0;
    }
    sprite->state = state;
}

void sprite_play_sequence(sprite_t *sprite, const int *sequence)
{
    sprite_set_sequence(sprite, sequence);
    sprite_set_state(sprite, SPRITE_PLAYING);
}

void sprite_loop_sequence(sprite_t *sprite, const int *sequence)
{
    sprite_set_sequence(sprite, sequence);
    sprite_set_state(sprite, SPRITE_LOOPING);
}

void sprite_play(sprite_t *sprite)
{
    sprite_set_state(sprite, SPRITE_PLAYING);
}

void sprite_loop(sprite_t *sprite)
{
    sprite_set_state(sprite, SPRITE_LOOPING);
}

void sprite_pause(sprite_t *sprite)
{
    sprite_set_state(sprite, SPRITE_PAUSED);
}

void sprite_stop(sprite_t *sprite)
{
    sprite->state = SPRITE_STOPPED;
    sprite->frame = 0;
    sprite->ticks = 0;
}

int sprite_is_playing(const sprite_t *sprite)
{
    return sprite->state == SPRITE_PLAYING;
}

int sprite_is_looping(const sprite_t *sprite)
{
    return sprite->state == SPRITE_LOOPING;
}

int sprite_is_paused(const sprite_t *sprite)
{
    return sprite->state == SPRITE_PAUSED;
}

int sprite_is_stopped(const sprite_t *sprite)
{
    return sprite->state == SPRITE_STOPPED;
}

int sprite_is_animating(const sprite_t *sprite)
{
    return sprite->state == SPRITE_PLAYING
        || sprite->state == SPRITE_LOOPING;
}

static int sprite_on_last_tick(const sprite_t *sprite)
{
    return sprite->ticks + 1 == sprite->delay;
}

static int sprite_on_last_frame(const sprite_t *sprite)
{
    if (sprite->sequence == NULL)
    {
        return sprite->frame + 1 == sprite->frames;
    }
    else
    {
        return sprite->sequence[sprite->frame + 1] < 0;
    }
}

static void sprite_update_frame(sprite_t *sprite)
{
    if (sprite_on_last_frame(sprite))
    {
        if (sprite->state == SPRITE_PLAYING)
        {
            sprite->state = SPRITE_STOPPED;
        }
        else
        {
            sprite->frame = 0;
        }
    }
    else
    {
        sprite->frame++;
    }
}

void render_draw_sprite(sprite_t *sprite)
{
    int index = sprite_get_index(sprite);
    int row = index / sprite->cols;
    int col = index % sprite->cols;

    SDL_Rect rect =
    {
        sprite->w * col,
        sprite->h * row,
        sprite->w,
        sprite->h
    };
    SDL_Rect area =
    {
        sprite->x,
        sprite->y,
        sprite->w,
        sprite->h
    };

    SDL_RenderCopy(
        renderer,
        sprite->texture,
        &rect,
        &area
    );
    if (sprite_is_animating(sprite))
    {
        if (sprite_on_last_tick(sprite))
        {
            sprite_update_frame(sprite);
            sprite->ticks = 0;
        }
        else
        {
            sprite->ticks++;
        }
    }
}

