#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

int main(void)
{
    char *str;

    str = string_clone("Hola mundo");
    printf("<%s>\n", str);
    free(str);

    str = string_print(" \n El valor de pi es %g \t ", 3.14);
    string_trim_inplace(str);
    printf("<%s>\n", str);
    free(str);

    str = string_trim(" \t En un lugar de la mancha ... \n ");
    printf("<%s>\n", str);
    free(str);

    putchar('>');
    str = file_get_line(stdin);
    printf("<%s>\n", str);
    free(str);

    const char *path = "wrong_file.txt";

    str = file_read(path);
    if (str == NULL)
    {
        perror("file_read");
        fprintf(stderr, "%s\n", path);
    }
    free(str);

    return 0;
}

