#include <stdlib.h>
#include "json.h"

int main(void)
{
    json *node = json_load_file("test.json");

    if (node == NULL)
    {
        exit(EXIT_FAILURE);
    }
    json_print(node);
    json_free(node);
    return 0;
}

