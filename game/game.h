#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define FPS 60

#define game_delay SDL_Delay

typedef SDL_Rect rect_t;
typedef SDL_Point point_t;
typedef SDL_Color color_t;
typedef SDL_Event event_t;

typedef int (callback_t)(int);

typedef struct
{
    TTF_Font *renderer;
    const char *name;
    int size;
    int w, h;
} font_t;

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    const char *title;
    int w, h;
    font_t font;
} game_t;

enum game_events
{
    EVENT_NONE = 0x00,
    EVENT_KEY_UP = 0x01,
    EVENT_KEY_DOWN = 0x02,
    EVENT_KEY_LEFT = 0x04,
    EVENT_KEY_RIGHT = 0x08,
    EVENT_QUIT = 0x10
};

enum game_states
{
    STATE_START,
    STATE_DRAW,
    STATE_STOP,
    STATES
};

void game_init(game_t *);
SDL_Keycode game_keydown(void);
void game_pause(void);

void render_set_color(const SDL_Color *);
void render_fill_rect(const SDL_Rect *);
void render_draw_rect(const SDL_Rect *);
void render_fill_area(int, int, int, int);
void render_draw_area(int, int, int, int);
void render_clear(void);
void render_present(void);

SDL_Texture *texture_create(const char *);

#endif /* GAME_H */

