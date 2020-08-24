#include <stddef.h>
#include "debug.h"
#include "hashmap.h"
#include "mapper.h"

static SDL_Renderer *renderer;
static hashmap *map;

enum
{
    RESOURCE_NONE,
    RESOURCE_IMAGE,
    RESOURCE_MIXER
};

typedef struct
{
    const char *path;
    int type;
    union
    {
        image_t *image;
        mixer_t *mixer;
    };
} resource_t;

static resource_t *mapper;

static resource_t *resource_create(void)
{
    resource_t *resource = calloc(1, sizeof *resource);

    if (resource == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    return resource;
}

static void resource_create_image(resource_t *resource)
{
    image_t *image = calloc(1, sizeof *image);

    if (image == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    SDL_Surface *surface = IMG_Load(resource->path);

    if (surface == NULL)
    {
        free(image);
        SDL_Log("IMG_Load: %s", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    image->texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (image->texture == NULL)
    {
        free(image);
        SDL_FreeSurface(surface);
        SDL_Log("SDL_CreateTextureFromSurface: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    image->w = surface->w;
    image->h = surface->h;
    SDL_FreeSurface(surface);

    resource->type = RESOURCE_IMAGE;
    resource->image = image;
}

static void resource_create_mixer(resource_t *resource)
{
    mixer_t *mixer = Mix_LoadWAV(resource->path);

    if (mixer == NULL)
    {
        SDL_Log("Mix_LoadWAV: %s", Mix_GetError());
        exit(EXIT_FAILURE);
    }
    resource->type = RESOURCE_MIXER;
    resource->mixer = mixer;
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

    switch (resource->type)
    {
        case RESOURCE_IMAGE:
            SDL_DestroyTexture(resource->image->texture);
            free(resource->image);
            break;
        case RESOURCE_MIXER:
            Mix_FreeChunk(resource->mixer);
            break;
    }
    free(resource);
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
    free(mapper);
}

void mapper_init(SDL_Renderer *this)
{
    renderer = this;
    atexit(map_destroy);
    map_create();
}

static resource_t *mapper_load_resource(const char *path)
{
    assert(path != NULL);

    mapper->path = path;

    resource_t *resource = hashmap_insert(map, mapper);

    if (resource == NULL)
    {
        perror("hashmap_insert");
        exit(EXIT_FAILURE);
    }
    return resource;
}

image_t *mapper_load_image(const char *path)
{
    resource_t *resource = mapper_load_resource(path);

    if (resource == mapper)
    {
        mapper = resource_create();
        resource_create_image(resource);
    }
    return resource->image;
}

mixer_t *mapper_load_mixer(const char *path)
{
    resource_t *resource = mapper_load_resource(path);

    if (resource == mapper)
    {
        mapper = resource_create();
        resource_create_mixer(resource);
    }
    return resource->mixer;
}

