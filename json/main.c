#include <stdio.h>
#include <stdlib.h>
#include "json.h"

int main(void)
{
    const char *path = "test.json";
    json *node = json_load_file(path);

    if (node == NULL)
    {
        fprintf(stderr, "Can not parse %s\n", path);
        exit(EXIT_FAILURE);
    }
    json_print(node);
    json_free(node);
    return 0;
}

