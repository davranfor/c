#ifndef BITMAP_H
#define BITMAP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

typedef struct
{
    SDL_Texture *texture;
    const char *path;
    int x, y, w, h;
} bitmap_t;

void bitmap_init(SDL_Renderer *);
bitmap_t *bitmap_load(const char *);
void bitmap_set_position(bitmap_t *, int , int );

void render_draw(const bitmap_t *);
void render_swap(const bitmap_t *, const bitmap_t *);
void render_clear_area(const bitmap_t *, int, int, int, int);
void render_clear_rect(const bitmap_t *, const SDL_Rect *);
void render_mod_color(bitmap_t *, Uint8);

typedef bitmap_t button_t;
int button_clicked(button_t *[], int);

#endif /* BITMAP_H */

