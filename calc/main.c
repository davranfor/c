#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ast.h"

int main(int argc, char *argv[])
{
    /*
        Example:
            3+4*2/(1-5)^2^3; gives: 3.00012 
        Launch from command line:
            echo "3+4*2/(1-5)^2^3;" | ./calculator
    */

    int print_tree = 0;

    if (argc == 2)
    {
        if (strcmp(argv[1], "--help") == 0)
        {
            ast_help();
            exit(EXIT_SUCCESS);
        }
        if (strcmp(argv[1], "--tree") == 0)
        {
            print_tree = 1;
        }
    }

    srand((unsigned)time(NULL));
    atexit(ast_destroy);

    if (ast_create() == 0)
    {
        perror("ast_create");
        exit(EXIT_FAILURE);
    }

    ast_node *ast;

    for (;;)
    {
        printf("> ");
        fflush(stdout);

        char str[1024];

        if (fgets(str, sizeof str, stdin) == NULL)
        {
            if (ferror(stdin))
            {
                perror("fgets");
                exit(EXIT_FAILURE);
            }
            else
            {
                puts("Bye");
                exit(EXIT_SUCCESS);
            }
        }
        ast = ast_build(str);
        if (ast == NULL)
        {
            exit(EXIT_FAILURE);
        }
        if (print_tree)
        {
            ast_print(ast);
        }
        else
        {
            ast_eval(ast);
        }
        ast_clean();
    }
    return 0;
}

