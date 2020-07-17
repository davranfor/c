#include <locale.h>
#include "gui.h"

int main(void)
{
    setlocale(LC_CTYPE, "");

    gui_app app =
    {
        .window =
        {
            .title = "Demo SDL2",
            .width = 500,
            .height = 500,
            .color = {232, 232, 232, 0}
        },
        .font =
        {
            .name = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            .size = 13
        }
    };

    gui_main(&app);
    return 0;
}

