#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include "utils.h"

/******************/
/* FILE UTILITIES */
/******************/

#define FGETLINE_MAX 256

FILE *file_open(const char *path, const char *mode)
{
    FILE *file = fopen(path, mode);

    if (file == NULL)
    {
        perror("fopen");
        fprintf(stderr, "%s\n", path);
    }
    return file;
}

long file_get_size(const char *path)
{
    FILE *file = file_open(path, "r");

    if (file == NULL)
    {
        return -1;
    }
    
    long size = fgetsize(file);

    fclose(file);
    return size;
}

char *file_read(const char *path)
{
    FILE *file = file_open(path, "r");

    if (file == NULL)
    {
        return NULL;
    }

    char *str = fgettext(file);

    fclose(file);
    return str;
}

size_t file_write(const char *path, const char *str, int append)
{
    FILE *file = file_open(path, append ? "a" : "w");

    if (file == NULL)
    {
        return 0;
    }

    size_t size = fsettext(file, str);

    fclose(file);
    return size;
}

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

char *fgetline(FILE *file)
{
    char str[FGETLINE_MAX];
    char *buf = NULL;
    char *ptr = NULL;
    size_t size = 0;

    while (fgets(str, sizeof str, file) != NULL)
    {
        size_t len = sizeof str;

        ptr = strchr(str, '\n');
        if (ptr != NULL)
        {
            len = (size_t)(ptr - str) + 1;
            *ptr = '\0';
        }
        ptr = realloc(buf, size + len);
        if (ptr == NULL)
        {
            free(buf);
            perror("malloc");
            return NULL;
        }
        memcpy(ptr + size, str, len);
        if (len != sizeof str)
        {
            return ptr;
        }
        size += len - 1;
        buf = ptr;
    }
    free(buf);
    perror("fgets");
    return NULL;
}

/********************/
/* STRING UTILITIES */
/********************/

char *dupstr(const char *str)
{
    assert(str != NULL);

    size_t size = strlen(str) + 1;
    char *ptr = malloc(size);

    if (ptr == NULL)
    {
        perror("malloc");
        return NULL;
    }
    return memcpy(ptr, str, size);
}

char *dupnstr(const char *str, size_t size)
{
    assert(str != NULL);

    char *ptr = malloc(size + 1);

    if (ptr == NULL)
    {
        perror("malloc");
        return NULL;
    }
    memcpy(ptr, str, size);
    ptr[size] = '\0';
    return ptr;
}

static char *vdupstrf(const char *fmt, va_list args)
{
    va_list copy;

    va_copy(copy, args);

    size_t len = (size_t)vsnprintf(NULL, 0, fmt, args);
    char *str = malloc(len + 1);

    if (str == NULL)
    {
        perror("malloc");
        return NULL;
    }
    vsprintf(str, fmt, copy);
    return str;
}

char *dupstrf(const char *fmt, ...)
{
    assert(fmt != NULL);

    va_list args;

    va_start(args, fmt);

    char *str = vdupstrf(fmt, args);

    va_end(args);
    return str;
}

static size_t lpos(const char *str)
{
    const char *end = str;

    while (isspace((unsigned char)*end))
    {
        end++;
    }
    return (size_t)(end - str);
}

static size_t rpos(const char *str, const char *end)
{
    const char *ptr = end;

    while (end > str)
    {
        if (!isspace((unsigned char)end[-1]))
        {
            break;
        }
        end--;
    }
    return (size_t)(ptr - end);
}

char *trim(const char *str)
{
    assert(str != NULL);
    str += lpos(str);

    size_t len = strlen(str);

    return dupnstr(str, len - rpos(str, str + len));
}

char *ltrim(const char *str)
{
    assert(str != NULL);
    str += lpos(str);
    return dupstr(str);
}

char *rtrim(const char *str)
{
    assert(str != NULL);

    size_t len = strlen(str);

    return dupnstr(str, len - rpos(str, str + len));
}

void trim_inplace(char *str)
{
    assert(str != NULL);

    char *ptr = str + lpos(str);

    size_t len = strlen(ptr);

    if (ptr != str)
    {
        memmove(str, ptr, len);
    }
    *(str + len - rpos(str, str + len)) = '\0';
}

void ltrim_inplace(char *str)
{
    assert(str != NULL);

    char *ptr = str + lpos(str);

    if (ptr != str)
    {
        size_t len = strlen(ptr);

        memmove(str, ptr, len);
        str[len] = '\0';
    }
}

void rtrim_inplace(char *str)
{
    assert(str != NULL);

    size_t len = strlen(str);

    *(str + len - rpos(str, str + len)) = '\0';
}

