#ifndef sdl_H
#define sdl_H

typedef struct
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} sdl_color;

typedef struct
{
    const char *title;
    int width;
    int height;
    sdl_color color;
} sdl_window;

typedef struct
{
    const char *name;
    int width;
    int height;
    int size;
} sdl_font;

typedef struct
{
    sdl_window window;
    sdl_font font;
} sdl_app;

void sdl_main(sdl_app *);

#endif /* sdl_H */

