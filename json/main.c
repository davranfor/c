#include <stdio.h>
#include <stdlib.h>
#include "file.h"
#include "json.h"

int main(void)
{
    char *text = file_read("ui.json");

    if (text == NULL)
    {
        fprintf(stderr, "Error reading ui.json\n");
        exit(EXIT_FAILURE);
    }

    json *node = json_create(text);

    free(text);
    if (node == NULL)
    {
        fprintf(stderr, "Error parsing ui.json\n");
        exit(EXIT_FAILURE);
    }
    json_print(node);
    json_free(node);
    return 0;
}

