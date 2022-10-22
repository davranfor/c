#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "json.h"
#include "json_schema.h"
#include "json_utils.h"

static json *parse(const char *path)
{
    json_error error; // Error handle is optional
    json *node = json_parse_file(path, &error);

    if (node == NULL)
    {
        json_raise_error(&error, path);
        exit(EXIT_FAILURE);
    }
    return node;
}

static int validate(const json *node, const char *path)
{
    json *schema = parse(path);

    puts("schema:");
    json_print(schema);

    int valid = schema_validate(node, schema, schema_default_callback, NULL);

    if (!valid)
    {
        fprintf(stderr, "'%s' doesn't validate\n", path);
    }
    json_free(schema);
    return valid;
}

static void print(const json *node)
{
    json_print(node);
}

static json *root = NULL;

static void clean(void)
{
    json_free(root);
}

int main(void)
{
    setlocale(LC_CTYPE, "");
    atexit(clean);

    root = parse("test.json");
    puts("json:");
    print(root);
    validate(root, "test.schema.json");
    return 0;
}

