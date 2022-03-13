/* 
 * Sends all nodes to a callback "func"
 * Exit when all nodes are read or when "func" returns a non 0 value
 */
static int callback(const json *node, void *data,
    int (*func)(const json *, void *), int level)
{
    if (node != NULL)
    {
        int result;

        if ((result = func(node, data)))
        {
            return result;
        }
        if ((result = callback(node->left, data, func, level + 1)))
        {
            return result;
        }
        if (level > 0)
        {
            if ((result = callback(node->right, data, func, level)))
            {
                return result;
            }
        }
    }
    return 0;
}

int json_callback(const json *node, void *data,
    int (*func)(const json *, void *))
{
    return callback(node, data, func, 0);
}

/*
 * Prints a tree recursively
 * "level" is incremented when a left branch is taken
 */
static void print(const json *node, int level)
{
    if (node != NULL)
    {
        print_opening(node, level);
        print(node->left, level + 1);
        print_closure(node, level);
        if (level > 0)
        {
            print(node->right, level);
        }
    }
}

void json_print(const json *node)
{
    print(node, 0);
}

void json_free(json *node)
{
    if (node != NULL)
    {
        json_free(node->left);
        json_free(node->right);
        free(node->name);
        free(node->value);
        free(node);
    }
}

