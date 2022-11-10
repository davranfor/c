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
    }
    return node;
}

static void validate(json *node, const char *path)
{
    json *schema = parse(path);

    if (schema == NULL)
    {
        return;
    }
    puts("json:");
    json_print(node);
    puts("schema.json:");
    json_print(schema);
    if (!json_validate(node, schema, json_schema_default_callback, NULL))
    {
        fprintf(stderr, "'%s' doesn't validate\n", path);
    }
    json_free(schema);
}

int main(void)
{
    setlocale(LC_CTYPE, "");

    json *node;

    if ((node = parse("test.json")))
    {
        validate(node, "test.schema.json");
        json_free(node);
    }
    return 0;
}

