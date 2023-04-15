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
        json_print_error(path, &error);
    }
    return node;
}

static void raise(const char *title, const json *node)
{
    char *path = json_path(node);

    if (path != NULL)
    {
        fprintf(stderr, "%s\n-> %s\n", title, path);
        free(path);
    }
    if (json_is_string(node))
    {
        char *str = json_encode_string(node);

        if (str != NULL)
        {
            fprintf(stderr, "-> \"%s\"\n", str);
            free(str);
        }
    }
    else if (json_is_scalar(node))
    {
        fprintf(stderr, "-> %s\n", json_string(node));
    }
}

static int callback(const json *node, const json *rule, int event, void *data)
{
    (void)data;

    const char *event_title[] = {"Warning", "Error", "Malformed schema"};

    fprintf(stderr, "\n[%s]\n", event_title[event]);
    raise("Testing", rule);
    raise("on node", node);
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
            fprintf(stderr, "\n'%s' doesn't validate\n", path);
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

