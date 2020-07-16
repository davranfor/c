#include <locale.h>
#include "gui.h"

int main(void)
{
    setlocale(LC_CTYPE, "");

    gui app =
    {
        .window.title = "Demo SDL2",
        .window.width = 640,
        .window.height = 480,
        .window.color = {232, 232, 232, 0},
        .font.name = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        .font.size = 13
    };

    gui_main(&app);
    return 0;
}

