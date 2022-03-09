static json *json_root(const json *node)
{
    json *root = NULL;

    if (node != NULL)
    {
        if (node->parent == NULL)
        {
            json *cast[1];

            /* silent compiler due to constant conversion */
            memcpy(cast, &node, sizeof node);
            return cast[0];
        }
        while (node->parent != NULL)
        {
            root = node->parent;
            node = node->parent;
        }
    }
    return root;
}
