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
    assert(frames > 0);
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
    sprite->delay = delay;
}

int sprite_get_delay(const sprite_t *sprite)
{
    return sprite->delay;
}

void sprite_set_sequence(sprite_t *sprite, const int *sequence)
{
    assert(*sequence > 0);
    sprite->sequence = sequence;
}

const int *sprite_get_sequence(const sprite_t *sprite)
{
    return sprite->sequence;
}

void sprite_play_sequence(sprite_t *sprite, const int *sequence)
{
    sprite_set_sequence(sprite, sequence);
    sprite->status = SPRITE_PLAYING;
    sprite->index = 0;
}

void sprite_loop_sequence(sprite_t *sprite, const int *sequence)
{
    sprite_set_sequence(sprite, sequence);
    sprite->status = SPRITE_LOOPING;
    sprite->index = 0;
}

void sprite_play(sprite_t *sprite)
{
    sprite->status = SPRITE_PLAYING;
}

void sprite_loop(sprite_t *sprite)
{
    sprite->status = SPRITE_LOOPING;
}

void sprite_stop(sprite_t *sprite)
{
    sprite->status = SPRITE_STOPPED;
}

void render_draw_sprite(sprite_t *sprite)
{
    if (sprite->index >= sprite->frames)
    {
        sprite->index = 0;
    }

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
    if (++sprite->count >= sprite->delay)
    {
        sprite->count = 0;
        sprite->index++;
    }
}

