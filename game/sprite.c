#include <assert.h>
#include <string.h>
#include "hashmap.h"
#include "sprite.h"

static SDL_Renderer *renderer;
static hashmap *map;
static sprite_t *mapper;

static sprite_t *new_sprite(void)
{
    sprite_t *sprite;

    sprite = calloc(1, sizeof *sprite);
    if (sprite == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    return sprite;
}

static int comp_sprite(const void *pa, const void *pb)
{
    const sprite_t *a = pa;
    const sprite_t *b = pb;

    return strcmp(a->path, b->path);
}

static unsigned long hash_sprite(const void *data)
{
    const sprite_t *sprite = data;

    return hash_str((const unsigned char *)sprite->path);
}

static void free_sprite(void *data)
{
    sprite_t *sprite = data;

    SDL_DestroyTexture(sprite->texture);
    free(sprite);
}

static void map_create(void)
{
    map = hashmap_create(comp_sprite, hash_sprite, 0);
    if (map == NULL)
    {
        perror("hashmap_create");
        exit(EXIT_FAILURE);
    }
    mapper = new_sprite();
}

static void map_destroy(void)
{
    hashmap_destroy(map, free_sprite);
    free(mapper);
}

void sprite_init(SDL_Renderer *this)
{
    renderer = this;
    map_create();
    atexit(map_destroy);
}

static void create_texture(sprite_t *sprite)
{
    SDL_Surface *surface = IMG_Load(sprite->path);

    if (surface == NULL)
    {
        SDL_Log("IMG_Load: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    sprite->texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (sprite->texture == NULL)
    {
        SDL_Log("SDL_CreateTextureFromSurface: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    sprite->surface.w = surface->w;
    sprite->surface.h = surface->h;
    SDL_FreeSurface(surface);
}

sprite_t *sprite_load(const char *path, int cols, int rows)
{
    assert(path != NULL);

    mapper->path = path;

    sprite_t *sprite = hashmap_insert(map, mapper);

    if (sprite == NULL)
    {
        perror("hashmap_insert");
        exit(EXIT_FAILURE);
    }
    if (sprite == mapper)
    {
        mapper = new_sprite();
        create_texture(sprite);
        sprite->rows = rows;
        sprite->cols = cols;
        sprite->w = sprite->surface.w / cols;
        sprite->h = sprite->surface.h / rows;
        sprite->delay = 10;
    }
    return sprite;
}

void sprite_set_position(sprite_t *sprite, int x, int y)
{
    sprite->x = x;
    sprite->y = y;
}

void sprite_set_delay(sprite_t *sprite, int delay)
{
    sprite->delay = delay;
}

int sprite_get_delay(const sprite_t *sprite)
{
    return sprite->delay;
}

void render_draw_sprite(sprite_t *sprite)
{
    int frames = sprite->cols * sprite->rows;

    if (sprite->index >= frames)
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

