#include <assert.h>
#include <string.h>
#include "hashmap.h"
#include "bitmap.h"

static SDL_Renderer *renderer;
static hashmap *map;
static bitmap_t *mapper;

static bitmap_t *new_bitmap(void)
{
    bitmap_t *bitmap;

    bitmap = calloc(1, sizeof *bitmap);
    if (bitmap == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    return bitmap;
}

static int comp_bitmap(const void *pa, const void *pb)
{
    const bitmap_t *a = pa;
    const bitmap_t *b = pb;

    return strcmp(a->path, b->path);
}

static unsigned long hash_bitmap(const void *data)
{
    const bitmap_t *bitmap = data;

    return hash_str((const unsigned char *)bitmap->path);
}

static void free_bitmap(void *data)
{
    bitmap_t *bitmap = data;

    SDL_DestroyTexture(bitmap->texture);
    free(bitmap);
}

static void map_create(void)
{
    map = hashmap_create(comp_bitmap, hash_bitmap, 0);
    if (map == NULL)
    {
        perror("hashmap_create");
        exit(EXIT_FAILURE);
    }
    mapper = new_bitmap();
}

static void map_destroy(void)
{
    hashmap_destroy(map, free_bitmap);
    free(mapper);
}

void bitmap_init(SDL_Renderer *this)
{
    renderer = this;
    map_create();
    atexit(map_destroy);
}

static void create_texture(bitmap_t *bitmap)
{
    SDL_Surface *surface = IMG_Load(bitmap->path);

    if (surface == NULL)
    {
        SDL_Log("IMG_Load: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    bitmap->texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (bitmap->texture == NULL)
    {
        SDL_Log("SDL_CreateTextureFromSurface: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    bitmap->w = surface->w;
    bitmap->h = surface->h;
    SDL_FreeSurface(surface);
}

bitmap_t *bitmap_load(const char *path)
{
    assert(path != NULL);

    mapper->path = path;

    bitmap_t *bitmap = hashmap_insert(map, mapper);

    if (bitmap == NULL)
    {
        perror("hashmap_insert");
        exit(EXIT_FAILURE);
    }
    if (bitmap == mapper)
    {
        mapper = new_bitmap();
        create_texture(bitmap);
    }
    return bitmap;
}

void bitmap_set_position(bitmap_t *bitmap, int x, int y)
{
    bitmap->x = x;
    bitmap->y = y;
}

void bitmap_mod_color(bitmap_t *bitmap, Uint8 mod)
{
    SDL_Rect area =
    {
        bitmap->x,
        bitmap->y,
        bitmap->w,
        bitmap->h
    };

    SDL_SetTextureColorMod(
        bitmap->texture,
        mod,
        mod,
        mod
    );
    SDL_RenderCopy(
        renderer,
        bitmap->texture,
        NULL,
        &area
    );
}

void render_draw_bitmap(const bitmap_t *bitmap)
{
    SDL_Rect area =
    {
        bitmap->x,
        bitmap->y,
        bitmap->w,
        bitmap->h
    };

    SDL_RenderCopy(
        renderer,
        bitmap->texture,
        NULL,
        &area
    );
}

static int button_match(button_t *button, int x, int y)
{
    if ((x >= button->x) && (x <= button->x + button->w) &&
        (y >= button->y) && (y <= button->y + button->h))
    {
        return 1;
    }
    return 0;
}

static void button_down(int index, button_t *button[])
{
    bitmap_mod_color(button[index - 1], 128);
    SDL_RenderPresent(renderer);
}

static void button_up(int index, button_t *button[])
{
    bitmap_mod_color(button[index - 1], 255);
    SDL_RenderPresent(renderer);
}

int button_clicked(button_t *button[], int buttons)
{
    SDL_MouseButtonEvent *mouse;
    SDL_Event event;
    int clicked = 0;
    int pressed = 0;
    int done = 0;

    while (!done)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_MOUSEBUTTONDOWN:
                    mouse = &event.button;
                    if ((mouse->button == SDL_BUTTON_LEFT) && (pressed == 0))
                    {
                        for (int index = 0; index < buttons; index++)
                        {
                            if (button_match(button[index], mouse->x, mouse->y))
                            {
                                clicked = index + 1;
                                button_down(clicked, button);
                                break;
                            }
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    mouse = &event.button;
                    if ((mouse->button == SDL_BUTTON_LEFT) && (clicked != 0))
                    {
                        button_up(clicked, button);
                        if (button_match(button[clicked - 1], mouse->x, mouse->y))
                        {
                            done = 1;
                            break;
                        }
                        clicked = 0;
                    }
                    break;
                case SDL_KEYDOWN:
                    if ((clicked != 0) || (pressed != 0))
                    {
                        break;
                    }
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_KP_ENTER:
                        case SDLK_RETURN:
                            pressed = 1;
                            if (buttons > 0)
                            {
                                button_down(pressed, button);
                            }
                            break;
                        case SDLK_ESCAPE:
                            pressed = 2;
                            if (buttons == 2)
                            {
                                button_down(pressed, button);
                            }
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    if (pressed == 0)
                    {
                        break;
                    }
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_KP_ENTER:
                        case SDLK_RETURN:
                            if (buttons > 0)
                            {
                                button_up(pressed, button);
                            }
                            done = 1;
                            break;
                        case SDLK_ESCAPE:
                            if (buttons == 2)
                            {
                                button_up(pressed, button);
                            }
                            done = 1;
                            break;
                    }
                    break;
                case SDL_QUIT:
                    if (clicked != 0)
                    {
                        button_up(clicked, button);
                    }
                    else if (pressed != 0)
                    {
                        button_up(pressed, button);
                    }
                    clicked = pressed = 0;
                    done = 1;
                    break;
            }
        }
        SDL_Delay(1);
    }
    if (clicked != 0)
    {
        return clicked;
    }
    if (pressed != 0)
    {
        return pressed;
    }
    return 0;
}

