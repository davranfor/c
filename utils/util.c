#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

char *dupstr(const char *str)
{
    size_t size = strlen(str) + 1;
    char *ptr = malloc(size);

    if (ptr == NULL)
    {
        perror("malloc");
        return NULL;
    }
    return memcpy(ptr, str, size);
}

char *dupstrf(const char *str, ...)
{
    (void)str;
    return NULL;
}

int main(void)
{
    char *str = dupstr("Hola mundo");

    puts(str);
    free(str);
    return 0;
}

