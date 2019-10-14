#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "utils.h"

int main(void)
{
    setlocale(LC_CTYPE, "");

    char *str;

    str = string_clone("Hello World!");
    if (str != NULL)
    {
        printf("<%s>\n", str);
        free(str);
    }

    str = string_print("The value of pi is %g", 3.14);
    if (str != NULL)
    {
        printf("<%s>\n", str);
        free(str);
    }

    str = string_trim(" \t En un lugar de la mancha ... \n ");
    if (str != NULL)
    {
        printf("<%s>\n", str);
        free(str);
    }

    str = "María";
    printf("\"%s\" -> length = %zu\n", str, string_length(str));

    const char *path = "test.txt";
    size_t size = file_write(path, "Enter text (Press CTRL + D to stop)\n>", FILE_TRUNCATE);

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
        free(str);
    }

    while ((str = file_read_line(stdin)))
    {
        printf("<%s>\n>", str);
        free(str);
    }
    if (file_error(stdin) != 0)
    {
        perror("file_read_line");
        fprintf(stderr, "File error #%d\n", file_error(stdin));
    }

    puts("\nBye!");
    return 0;
}

