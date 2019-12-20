#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils.h"
#include "ast.h"

static char *script;

static void script_destroy(void)
{
    free(script);
}

int main(int argc, char *argv[])
{
    int explain = 0;

    if (argc == 2)
    {
        if (strcmp(argv[1], "--help") == 0)
        {
            ast_help();
            exit(EXIT_SUCCESS);
        }
        if (strcmp(argv[1], "--tree") == 0)
        {
            explain = 1;
        }
    }
    if ((argc > 1 ) && (explain == 0))
    {
        fprintf(stderr, "%s: Unrecognized command line: %s\n",
                argv[0],
                argv[1]
        );
        exit(EXIT_FAILURE);
    }

    srand((unsigned)time(NULL));

    const char *path = "script.py";

    script = file_read_with_prefix(path, " ");
    if (script == NULL)
    {
        perror("file_read");
        fprintf(stderr, "%s\n", path);
        exit(EXIT_FAILURE);
    }

    atexit(script_destroy);
    atexit(ast_destroy);

    ast_create();

    ast_node *ast = ast_build(script);

    if (ast == NULL)
    {
        exit(EXIT_FAILURE);
    }
    //explain = 1;
    if (explain != 0)
    {
        ast_explain(ast);
    }
    ast_eval(ast);
    return 0;
}

