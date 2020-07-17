#ifndef GUI_H
#define GUI_H

typedef void *callback_ptr(void *);

typedef struct
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} gui_color;

typedef struct
{
    const char *title;
    int width;
    int height;
    gui_color color;
} gui_window;

typedef struct
{
    const char *name;
    int width;
    int height;
    int size;
} gui_font;

typedef struct
{
    gui_window window;
    gui_font font;
} gui_app;

void gui_main(gui_app *);

#endif /* GUI_H */

