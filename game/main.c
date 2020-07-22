#include <locale.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "types.h"
#include "game.h"

static game_t game =
{
    .title = "Mirlo cagón vs. Perro flauta",
    .width = 1200,
    .height = 650,
    .font =
    {
        .name = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        .size = 13
    }
};

int main(void)
{
    setlocale(LC_CTYPE, "");
    srand((unsigned)time(NULL));
    game_run(&game);
    return 0;
}

