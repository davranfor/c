#include <stdio.h>
#include "chrono.h"

int main(void)
{
    struct chrono chrono;

    puts("Testing chrono");
    chrono_start(&chrono);
    millisleep(1000);
    chrono_stop(&chrono);
    millisleep(100);
    chrono_resume(&chrono);
    millisleep(234);
    printf("chrono: %ld\n", chrono_elapsed(&chrono));
    return 0;
}

