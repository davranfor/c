#include <locale.h>
#include <time.h>
#include "game.h"
#include "load.h"
#include "loop.h"

static game_t game =
{
    .title = "Mirlo cagón vs. Perro flauta",
    .w = 1200,
    .h = 650
};

int main(void)
{
    setlocale(LC_CTYPE, "");
    srand((unsigned)time(NULL));
    game_load(&game);
    game_init(&game);
    game_loop(&game);
    return 0;
}

