#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json.h"

static char *file_read(const char *path)
{
    FILE *file;
    long size;
    char *str;

    file = fopen(path, "rb");
    if (file == NULL) {
        perror("fopen");
        return NULL;
    }
    if (fseek(file, 0, SEEK_END) == -1) {
        perror("fseek");
        return NULL;
    }
    size = ftell(file);
    if (size == -1) {
        perror("ftell");
        return NULL;
    }
    if (fseek(file, 0, SEEK_SET) == -1) {
        perror("fseek");
        return NULL;
    }
    str = malloc((size_t)size + 1);
    if (str == NULL) {
        return NULL;
    }
    if (fread(str, 1, (size_t)size, file) != (size_t)size) {
        perror("fread");
        return NULL;
    }
    str[size] = '\0';
    fclose(file);
    return str;
}

int main(void)
{
    json *node;
    char *text;

    text = file_read("ui.json");
    if (text == NULL) {
        fprintf(stderr, "Error reading ui.json\n");
        exit(EXIT_FAILURE);
    }
    node = json_create(text);
    free(text);
    if (node == NULL) {
        fprintf(stderr, "Error parsing ui.json\n");
        exit(EXIT_FAILURE);
    }
    json_print(node);
    json_free(node);
    return 0;
}

