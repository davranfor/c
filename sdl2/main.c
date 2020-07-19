#include <locale.h>
#include <stdlib.h>
#include <time.h>
#include "sdl.h"

int main(void)
{
    setlocale(LC_CTYPE, "");
    srand((unsigned)time(NULL));

    sdl_app app =
    {
        .window =
        {
            .title = "Demo SDL2",
            .width = 500,
            .height = 500,
            .color = {218, 218, 218, 255}
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

