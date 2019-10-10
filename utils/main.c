#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

int main(void)
{
    char *str;

    str = dupstr("Hola mundo");
    printf("<%s>\n", str);
    free(str);

    str = dupstrf(" \n El valor de pi es %g \t ", 3.14);
    trim_inplace(str);
    printf("<%s>\n", str);
    free(str);

    str = trim(" \t En un lugar de la mancha ... \n ");
    printf("<%s>\n", str);
    free(str);

    putchar('>');
    str = fgetline(stdin);
    printf("<%s>\n", str);
    free(str);
    return 0;
}

