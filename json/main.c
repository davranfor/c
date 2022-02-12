#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "json.h"

static char *read(FILE *file, size_t size)
{
    char *str = malloc(size + 1);

    if (str != NULL)
    {
        if (fread(str, 1, size, file) == size)
        {
            str[size] = '\0';
        }
        else
        {
            free(str);
            str = NULL;
        }
    }
    return str;
}

static char *read_file(const char *path)
{
    FILE *file = fopen(path, "rb");

    if (file == NULL)
    {
        return NULL;
    }

    char *str = NULL;

    if (fseek(file, 0L, SEEK_END) != -1)
    {
        long size = ftell(file);

        if ((size != -1) && (fseek(file, 0L, SEEK_SET) != -1))
        {
            str = read(file, (size_t)size);
        }
    }
    fclose(file);
    return str;
}

static json *read_json(const char *path)
{
    char *str = read_file(path);

    if (str == NULL)
    {
        perror("read_file");
        exit(EXIT_FAILURE);
    }

    json_error error;
    json *node = json_parse(str, &error); // Pass error as NULL to skip errors

    free(str);
    if (node == NULL)
    {
        fprintf(stderr, "json_parse: Error at line %d, column %d\n",
                error.line, error.column
        );
        exit(EXIT_FAILURE);
    }
    return node;
}

int main(void)
{
    setlocale(LC_CTYPE, "");

    json *node = read_json("test.json");

    json_print(node);
    json_free(node);
    return 0;
}

