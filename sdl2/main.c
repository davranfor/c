#include <locale.h>
#include "sdl.h"

int main(void)
{
    setlocale(LC_CTYPE, "");

    sdl_app app =
    {
        .window =
        {
            .title = "Demo SDL2",
            .width = 500,
            .height = 500,
            .color = {232, 232, 232, 255}
        },
        .font =
        {
            .name = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            .size = 13
        }
    };

    sdl_main(&app);
    return 0;
}

