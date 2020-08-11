#ifndef SPRITE_H
#define SPRITE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

typedef struct
{
    struct {int w, h;} surface;
    SDL_Texture *texture;
    const char *path;
    int x, y, w, h;
    int rows, cols;
    int index;
    int delay;
    int count;
} sprite_t;

void sprite_init(SDL_Renderer *);
sprite_t *sprite_load(const char *, int, int);
void sprite_set_position(sprite_t *, int , int);
void sprite_set_delay(sprite_t *, int);
int sprite_get_delay(const sprite_t *);

void render_draw_sprite(sprite_t *);

#endif /* SPRITE_H */

