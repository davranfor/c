#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"

long fgetsize(FILE *file)
{
    if (fseek(file, 0L, SEEK_END) == -1)
    {
        perror("fseek");
        return -1;
    }

    long size = ftell(file);

    if (size == -1)
    {
        perror("ftell");
        return -1;
    }
    if (fseek(file, 0L, SEEK_SET) == -1)
    {
        perror("fseek");
        return -1;
    }
    return size;
}

static char *gettext(FILE *file, size_t size)
{
    char *str = malloc(size + 1);

    if (str == NULL)
    {
        perror("malloc");
        return NULL;
    }
    if (fread(str, 1, size, file) != size)
    {
        free(str);
        perror("fread");
        return NULL;
    }
    str[size] = '\0';
    return str;
}

char *fgettext(FILE *file)
{
    long size = fgetsize(file);

    if (size == -1)
    {
        return NULL;
    }    
    return gettext(file, (size_t)size);
}

size_t fsettext(FILE *file, const char *str)
{
    size_t size = strlen(str);

    if (fwrite(str, 1, size, file) != size)
    {
        perror("fwrite");
        return 0;
    }
    return size;
}

long file_get_size(const char *path)
{
    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        perror("fopen");
        return -1;
    }
    
    long size = fgetsize(file);

    fclose(file);
    return size;
}

char *file_read(const char *path)
{
    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        perror("fopen");
        return NULL;
    }

    char *str = fgettext(file);

    fclose(file);
    return str;
}

size_t file_write(const char *path, const char *str, int append)
{
    FILE *file = fopen(path, append ? "a" : "w");

    if (file == NULL)
    {
        perror("fopen");
        return 0;
    }

    size_t size = fsettext(file, str);

    fclose(file);
    return size;
}

