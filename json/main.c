#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "json.h"

static json *parse(const char *path)
{
    char *text = file_read(path);

    if (text == NULL)
    {
        fprintf(stderr, "Can't read %s\n", path);
        exit(EXIT_FAILURE);
    }

    const char *error = NULL;
    json *node = json_parse(text, &error);

    if (node == NULL)
    {
        json_print_error(text, error);
        fprintf(stderr, "Can't parse %s\n", path);
        free(text);
        exit(EXIT_FAILURE);
    }
    else
    {
        free(text);
    }
    return node;
}

int main(void)
{
    json *node = parse("test.json");

    json_print(node);
    json_free(node);
    return 0;
}

