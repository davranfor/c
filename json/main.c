#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "json.h"

static char *read_file(const char *path)
{
    FILE *file = fopen(path, "rb");

    if (file == NULL)
    {
        return NULL;
    }

    char *str = NULL;

    do
    {
        if (fseek(file, 0L, SEEK_END) == -1)
        {
            break;
        }

        long tell = ftell(file);

        if ((tell == -1) || (fseek(file, 0L, SEEK_SET) == -1))
        {
            break;
        }

        size_t size = (size_t)tell;

        str = malloc(size + 1);
        if (str != NULL)
        {
            if (fread(str, 1, size, file) == size)
            {
                str[size] = '\0';
            }
            else
            {
                free(str);
            }
        }
    }
    while (0);
    fclose(file);
    return str;
}

static json *read_json(const char *path)
{
    char *text = read_file(path);

    if (text == NULL)
    {
        perror("read_json");
        exit(EXIT_FAILURE);
    }

    json *node = json_parse(text);

    free(text);
    return node;
}

int main(void)
{
    setlocale(LC_CTYPE, "");

    const char *path = "test.json";
    json *node = read_json(path);

    if (node == NULL)
    {
        perror("read_json");
        exit(EXIT_FAILURE);
    }
    json_print(node);
    json_free(node);
    return 0;
}

