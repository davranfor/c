#include <stdio.h>
#include <stdlib.h>
#include "json.h"
#include "json_utils.h"

static char *read_file(FILE *file, size_t size)
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

char *json_read_file(const char *path)
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
            str = read_file(file, (size_t)size);
        }
    }
    fclose(file);
    return str;
}

json *json_parse_file(const char *path, json_error *error)
{
    if (error)
    {
        error->file = 0;
        error->line = 0;
        error->column = 0;
    }

    char *str = json_read_file(path);

    if (str == NULL)
    {
        if (error)
        {
            error->file = 1;
        }
        return NULL;
    }

    json *node = json_parse(str, error);

    free(str);
    return node;
}

