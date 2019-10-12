#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

int main(void)
{
    char *str;

    str = string_clone("Hello World!");
    if (str != NULL)
    {
        printf("<%s>\n", str);
    }
    free(str);

    str = string_print("The value of pi is %g", 3.14);
    if (str != NULL)
    {
        printf("<%s>\n", str);
    }
    free(str);

    str = string_trim(" \t En un lugar de la mancha ... \n ");
    if (str != NULL)
    {
        printf("<%s>\n", str);
    }
    free(str);

    const char *path = "test.txt";
    size_t size = file_write(path, "Enter text\n>", FILE_TRUNCATE);

    if (size == FILE_WRITE_ERROR)
    {
        perror("file_write");
        fprintf(stderr, "%s\n", path);
    }

    str = file_read(path);
    if (str == NULL)
    {
        perror("file_read");
        fprintf(stderr, "%s\n", path);
    }
    else
    {
        printf("%s", str);
    }
    free(str);

    str = file_read_line(stdin);
    if (str != NULL)
    {
        printf("<%s>\n", str);
    }
    free(str);

    return 0;
}

