#include <locale.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"
#include "sdl.h"

int main(void)
{
    setlocale(LC_CTYPE, "");
    srand((unsigned)time(NULL));

    sdl_game game =
    {
        .title = "Mirlo cagón vs. Perro flauta",
        .width = 1200,
        .height = 650,
        .color = {255, 255, 255, 0},
        .font =
        {
            .name = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            .size = 13
        }
    };

    sdl_main(&game);
    return 0;
}

