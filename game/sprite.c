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

    resource_t *resource = mapper_load_resource(path);
    sprite_t *sprite = calloc(1, sizeof *sprite);

    if (sprite == NULL)
    {
        perror("sprite_create");
        exit(EXIT_FAILURE);
    }
    sprite->texture = resource->texture;
    sprite->w = resource->w / cols;
    sprite->h = resource->h / rows;
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

void sprite_set_index(sprite_t *sprite, int index)
{
    sprite->index = index;
}

int sprite_get_index(const sprite_t *sprite)
{
    return sprite->index;
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
        sprite->index = 0;
        sprite->count = 0;
    }
}

const int *sprite_get_sequence(const sprite_t *sprite)
{
    return sprite->sequence;
}

void sprite_play_sequence(sprite_t *sprite, const int *sequence)
{
    sprite_set_sequence(sprite, sequence);
    sprite_play(sprite);
}

void sprite_loop_sequence(sprite_t *sprite, const int *sequence)
{
    sprite_set_sequence(sprite, sequence);
    sprite_loop(sprite);
}

void sprite_play(sprite_t *sprite)
{
    if (sprite->state == SPRITE_STOPPED)
    {
        sprite->index = 0;
        sprite->count = 0;
    }
    sprite->state = SPRITE_PLAYING;
}

void sprite_loop(sprite_t *sprite)
{
    if (sprite->state == SPRITE_STOPPED)
    {
        sprite->index = 0;
        sprite->count = 0;
    }
    sprite->state = SPRITE_LOOPING;
}

void sprite_stop(sprite_t *sprite)
{
    sprite->state = SPRITE_STOPPED;
    sprite->index = 0;
    sprite->count = 0;
}

void sprite_pause(sprite_t *sprite)
{
    sprite->state = SPRITE_PAUSED;
}

int sprite_is_playing(sprite_t *sprite)
{
    return sprite->state == SPRITE_PLAYING;
}

int sprite_is_looping(sprite_t *sprite)
{
    return sprite->state == SPRITE_LOOPING;
}

int sprite_is_stopped(sprite_t *sprite)
{
    return sprite->state == SPRITE_STOPPED;
}

int sprite_is_paused(sprite_t *sprite)
{
    return sprite->state == SPRITE_PAUSED;
}

int sprite_is_animating(sprite_t *sprite)
{
    return sprite->state == SPRITE_PLAYING
        || sprite->state == SPRITE_LOOPING;
}

void render_draw_sprite(sprite_t *sprite)
{
    int row = sprite->index / sprite->cols;
    int col = sprite->index % sprite->cols;

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
    if (!sprite_is_animating(sprite))
    {
        return;
    }
    if (sprite->count + 1 == sprite->delay)
    {
        if (sprite->index + 1 == sprite->frames)
        {
            if (sprite->state == SPRITE_PLAYING)
            {
                sprite->state = SPRITE_STOPPED;
                return;
            }
            sprite->index = 0;
        }
        else
        {
            sprite->index++;
        }
        sprite->count = 0;
    }
    else
    {
        sprite->count++;
    }
}

