#ifndef SPRITE_H
#define SPRITE_H

#define SPRITE(sprite) sprites[SPRITE_##sprite]

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

typedef struct
{
    SDL_Texture *texture;
    const int *sequence;
    int x, y, w, h;
    int rows, cols;
    int frames;
    int frame;
    int delay;
    int ticks;
    int state;
} sprite_t;

enum sprite_state
{
    SPRITE_STOPPED,
    SPRITE_PLAYING,
    SPRITE_LOOPING,
    SPRITE_PAUSED
};

void sprite_init(SDL_Renderer *);
sprite_t *sprite_create(const char *, int, int);
void sprite_destroy(sprite_t *);
void sprite_set_position(sprite_t *, int , int);
void sprite_get_position(const sprite_t *, int *, int *);
void sprite_set_frames(sprite_t *, int);
int sprite_get_frames(const sprite_t *);
void sprite_set_frame(sprite_t *, int);
int sprite_get_frame(const sprite_t *);
int sprite_get_index(const sprite_t *);
void sprite_set_delay(sprite_t *, int);
int sprite_get_delay(const sprite_t *);
void sprite_set_sequence(sprite_t *, const int *);
const int *sprite_get_sequence(const sprite_t *);
void sprite_play_sequence(sprite_t *, const int *);
void sprite_loop_sequence(sprite_t *, const int *);
void sprite_play(sprite_t *);
void sprite_loop(sprite_t *);
void sprite_pause(sprite_t *);
void sprite_stop(sprite_t *);
int sprite_is_playing(const sprite_t *);
int sprite_is_looping(const sprite_t *);
int sprite_is_paused(const sprite_t *);
int sprite_is_stopped(const sprite_t *);
int sprite_is_animating(const sprite_t *);

void render_draw_sprite(sprite_t *);

#endif /* SPRITE_H */

