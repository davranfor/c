#include <locale.h>
#include <time.h>
#include "game.h"
#include "init.h"

static game_t game =
{
    .title = "Mirlo cagón vs. Perro flauta",
    .width = 1200,
    .height = 650,
    .font =
    {
        .name = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        .size = 40
    }
};

int main(void)
{
    setlocale(LC_CTYPE, "");
    srand((unsigned)time(NULL));
    game_init(&game);
    return 0;
}

