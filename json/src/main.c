#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "json.h"
#include "json_schema.h"

static json *parse(const char *path)
{
    json_error error; // Error handle is optional
    json *node = json_parse_file(path, &error);

    if (node == NULL)
    {
        json_raise_error(path, &error);
    }
    return node;
}

static int callback(const json *node, const json *rule, int depth, void *data)
{
    (void)depth;
    (void)data;

    char *path;

    path = json_path(rule);
    if (path != NULL)
    {
        fprintf(stderr, "Testing: %s\n", path);
        free(path);
    }
    if (json_string(rule))
    {
        fprintf(stderr, "Expected: %s\n", json_string(rule));
    }
    if (json_string(node))
    {
        fprintf(stderr, "Got: %s\n", json_string(node));
    }
    path = json_path(node);
    if (path != NULL)
    {
        fprintf(stderr, "At: %s\n", path);
        free(path);
    }
    return 1;
}

static void validate(const json *node, const char *path)
{
    json *schema = parse(path);

    if (schema != NULL)
    {
        puts("json:");
        json_print(node);
        puts("schema.json:");
        json_print(schema);
        if (!json_validate(node, schema, callback, NULL))
        {
            fprintf(stderr, "'%s' doesn't validate\n", path);
        }
        json_free(schema);
    }
}

int main(void)
{
    setlocale(LC_CTYPE, "");

    json *node = parse("test.json");

    if (node != NULL)
    {
        validate(node, "test.schema.json");
        json_free(node);
    }
    return 0;
}

