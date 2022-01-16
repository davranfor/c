/*! 
 *  \brief     SplayTree
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include "splaytree.h"

struct node
{
    void *data;
    struct node *left, *right;
};

struct splaytree
{
    int (*comp)(const void *, const void *);
    struct node *root;
};

splaytree *splaytree_create(int (*comp)(const void *, const void *))
{
    splaytree *tree = calloc(1, sizeof(*tree));

    if (tree != NULL)
    {
        tree->comp = comp;
    }
    return tree;
}

static struct node *splay(struct node *root, const void *data,
    int (*func)(const void *, const void *))
{
    struct node node, *left, *right, *temp;
    int comp;

    node.left = node.right = NULL;
    left = right = &node;
    for (;;)
    {
        comp = func(data, root->data);
        if (comp < 0)
        {
            if (root->left == NULL)
            {
                break;
            }
            if (func(data, root->left->data) < 0)
            {
                temp = root->left;
                root->left = temp->right;
                temp->right = root;
                root = temp;
                if (root->left == NULL)
                {
                    break;
                }
            }
            right->left = root;
            right = root;
            root = root->left;
        }
        else if (comp > 0)
        {
            if (root->right == NULL)
            {
                break;
            }
            if (func(data, root->right->data) > 0)
            {
                temp = root->right;
                root->right = temp->left;
                temp->left = root;
                root = temp;
                if (root->right == NULL)
                {
                    break;
                }
            }
            left->right = root;
            left = root;
            root = root->right;
        }
        else
        {
            break;
        }
    }
    left->right = root->left;
    right->left = root->right;
    root->left = node.right;
    root->right = node.left;
    return root;
}

void *splaytree_insert(splaytree *tree, void *data)
{
    struct node **root = &tree->root;
    struct node *node, *temp;
    int comp = 0;

    if (*root != NULL)
    {
        node = splay(*root, data, tree->comp);
        comp = tree->comp(data, node->data);
        if (comp == 0)
        {
            *root = node;
            return node->data;
        }
    }
    temp = malloc(sizeof(*temp));
    if (temp == NULL)
    {
        return NULL;
    }
    if (comp == 0)
    {
        temp->left = temp->right = NULL;
    }
    else
    {
        if (comp < 0)
        {
            temp->left = node->left;
            temp->right = node;
            node->left = NULL;
        }
        else
        {
            temp->right = node->right;
            temp->left = node;
            node->right = NULL;
        }
    }
    *root = temp;
    temp->data = data;
    return data;
}

void *splaytree_delete(splaytree *tree, const void *data)
{
    struct node **root = &tree->root;

    if (*root == NULL)
    {
        return NULL;
    }

    struct node *node;

    node = splay(*root, data, tree->comp);
    if (tree->comp(data, node->data) == 0)
    {
        struct node *temp;

        if (node->left == NULL)
        {
            temp = node->right;
        }
        else
        {
            temp = splay(node->left, data, tree->comp);
            temp->right = node->right;
        }
        *root = temp;

        void *res = node->data;

        free(node);
        return res;
    }
    else
    {
        *root = node;
        return NULL;
    }
}

void *splaytree_search(splaytree *tree, const void *data)
{
    struct node **root = &tree->root;

    if (*root == NULL)
    {
        return NULL;
    }
    *root = splay(*root, data, tree->comp);
    if (tree->comp(data, (*root)->data) == 0)
    {
        return (*root)->data;
    }
    else
    {
        return NULL;
    }
}

void *splaytree_walk(const splaytree *tree, void *(*func)(void *, void *), void *cookie)
{
    struct node *root = tree->root;
    struct node *node;

    void *result = NULL;
 
    while (root != NULL)
    {
        if (root->left == NULL)
        {
            if (result == NULL)
            {
                result = func(root->data, cookie);
            }
            root = root->right;
        }
        else
        {
            node = root->left;
            while (node->right != NULL && node->right != root)
            {
                node = node->right;
            }
            if (node->right == NULL)
            {
                node->right = root;
                root = root->left;
            }
            else
            {
                node->right = NULL;
                if (result == NULL)
                {
                    result = func(root->data, cookie);
                }
                root = root->right;
            }
        }
    }
    return result;
}

void splaytree_destroy(splaytree *tree, void (*func)(void *))
{
    struct node *root = tree->root;
    struct node *node;

    while (root != NULL)
    {
        if (root->left != NULL)
        {
            node = root->left;
            root->left = node->right;
            node->right = root;
        }
        else
        {
            node = root->right;
            if (func != NULL)
            {
                func(root->data);
            }
            free(root);
        }
        root = node;
    }
    free(tree);
}

