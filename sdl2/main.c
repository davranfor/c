#include <locale.h>
#include "gui.h"

int main(void)
{
    setlocale(LC_CTYPE, "");

    gui_app app =
    {
        .window.title = "Demo SDL2",
        .window.width = 500,
        .window.height = 500,
        .window.color = {232, 232, 232, 0},
        .font.name = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        .font.size = 13
    };

    gui_main(&app);
    return 0;
}

