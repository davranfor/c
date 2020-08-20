#include <stddef.h>
#include "debug.h"
#include "hashmap.h"
#include "mapper.h"

static SDL_Renderer *renderer;
static resource_t *mapper;
static hashmap *map;

static resource_t *resource_create(void)
{
    resource_t *resource = SDL_calloc(1, sizeof *resource);

    if (resource == NULL)
    {
        perror("SDL_calloc");
        exit(EXIT_FAILURE);
    }
    return resource;
}

static void resource_fill(resource_t *resource)
{
    SDL_Surface *surface = IMG_Load(resource->path);

    if (surface == NULL)
    {
        SDL_Log("IMG_Load: %s", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    resource->texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (resource->texture == NULL)
    {
        SDL_Log("SDL_CreateTextureFromSurface: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    resource->w = surface->w;
    resource->h = surface->h;
    SDL_FreeSurface(surface);
}

static int resource_comp(const void *pa, const void *pb)
{
    const resource_t *a = pa;
    const resource_t *b = pb;

    return SDL_strcmp(a->path, b->path);
}

static unsigned long resource_hash(const void *data)
{
    const resource_t *resource = data;

    return hash_str((const unsigned char *)resource->path);
}

static void resource_destroy(void *data)
{
    resource_t *resource = data;

    SDL_DestroyTexture(resource->texture);
    SDL_free(resource);
}

static void map_create(void)
{
    map = hashmap_create(resource_comp, resource_hash, 0);
    if (map == NULL)
    {
        perror("hashmap_create");
        exit(EXIT_FAILURE);
    }
    mapper = resource_create();
}

static void map_destroy(void)
{
    hashmap_destroy(map, resource_destroy);
    SDL_free(mapper);
}

void mapper_init(SDL_Renderer *this)
{
    renderer = this;
    atexit(map_destroy);
    map_create();
}

resource_t *mapper_load_resource(const char *path)
{
    assert(path != NULL);

    mapper->path = path;

    resource_t *resource = hashmap_insert(map, mapper);

    if (resource == NULL)
    {
        perror("hashmap_insert");
        exit(EXIT_FAILURE);
    }
    if (resource == mapper)
    {
        mapper = resource_create();
        resource_fill(resource);
    }
    return resource;
}

