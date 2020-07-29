#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define FPS 60

typedef struct
{
    TTF_Font *renderer;
    const char *name;
    int size;
    int w, h;
} font_t;

typedef struct
{
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    const char *title;
    int width, height;
    font_t font;
} game_t;

enum game_states
{
    STATE_START,
    STATE_DRAW,
    STATE_STOP,
    STATES
};

typedef int (callback_t)(game_t *);

void game_set(game_t *);
game_t *game_get(void);
SDL_Keycode game_keydown(void);
void game_pause(void);
void game_quit(void);

typedef struct
{
    SDL_Texture *texture;
    int x, y, w, h;
} bitmap_t;

void bitmap_load(bitmap_t *, const char *);
void bitmap_text(bitmap_t *, const char *);

int button_clicked(bitmap_t *[], int);

#endif /* GAME_H */

