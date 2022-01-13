/*! 
 *  \brief     Red-Black tree
 *  \author    David Ranieri <davranfor@gmail.com>
 */

/*
 * Adapted from the following code written by Todd C. Miller:
 * https://opensource.apple.com/source/sudo/sudo-77/src
 *
 * Copyright (c) 2004-2005, 2007,2009 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include "rbtree.h"

/*
 * Red-Black tree, see http://en.wikipedia.org/wiki/Red-black_tree
 *
 * A red-black tree is a binary search tree where each node has a color
 * attribute, the value of which is either red or black.  Essentially, it
 * is just a convenient way to express a 2-3-4 binary search tree where
 * the color indicates whether the node is part of a 3-node or a 4-node.
 * In addition to the ordinary requirements imposed on binary search
 * trees, we make the following additional requirements of any valid
 * red-black tree:
 *  1) Every node is either red or black.
 *  2) The root is black.
 *  3) All leaves are black.
 *  4) Both children of each red node are black.
 *  5) The paths from each leaf up to the root each contain the same
 *     number of black nodes.
 */

enum rbcolor
{
    red,
    black
};

struct node
{
    struct node *left, *right, *parent;
    void *data;
    enum rbcolor color;
};

struct rbtree
{
    int (*comp)(const void *, const void *);
    struct node root;
    struct node nil;
};

#define rbnil(t) (&(t)->nil)
#define rbroot(t) (&(t)->root)
#define rbfirst(t) ((t)->root.left)

/*
 * Create a red black tree struct using the specified compare routine.
 * Allocates and returns the initialized (empty) tree.
 */
rbtree *rbtree_create(int (*comp)(const void *, const void *))
{
    rbtree *tree = malloc(sizeof *tree);

    if (tree == NULL)
    {
        return NULL;
    }
    tree->comp = comp;

    /*
     * We use a self-referencing sentinel node called nil to simplify the
     * code by avoiding the need to check for NULL pointers.
     */
    tree->nil.left = tree->nil.right = tree->nil.parent = &tree->nil;
    tree->nil.color = black;
    tree->nil.data = NULL;

    /*
     * Similarly, the fake root node keeps us from having to worry
     * about splitting the root.
     */
    tree->root.left = tree->root.right = tree->root.parent = &tree->nil;
    tree->root.color = black;
    tree->root.data = NULL;

    return tree;
}

/*
 * Perform a left rotation starting at node.
 */
static void rotate_left(rbtree *tree, struct node *node)
{
    struct node *child;

    child = node->right;
    node->right = child->left;
    if (child->left != rbnil(tree))
    {
        child->left->parent = node;
    }
    child->parent = node->parent;
    if (node == node->parent->left)
    {
	    node->parent->left = child;
    }
    else
    {
	    node->parent->right = child;
    }
    child->left = node;
    node->parent = child;
}

/*
 * Perform a right rotation starting at node.
 */
static void rotate_right(rbtree *tree, struct node *node)
{
    struct node *child;

    child = node->left;
    node->left = child->right;
    if (child->right != rbnil(tree))
    {
        child->right->parent = node;
    }
    child->parent = node->parent;
    if (node == node->parent->left)
    {
	    node->parent->left = child;
    }
    else
    {
	    node->parent->right = child;
    }
    child->right = node;
    node->parent = child;
}

/*
 * Insert data pointer into a redblack tree.
 * Returns a NULL pointer on success.  If a node matching "data"
 * already exists, a pointer to the existant node is returned.
 */
void *rbtree_insert(rbtree *tree, void *data)
{
    struct node *node = rbfirst(tree);
    struct node *parent = rbroot(tree);

    /* Find correct insertion point. */
    while (node != rbnil(tree))
    {
	    parent = node;

        int comp;

	    if ((comp = tree->comp(data, node->data)) == 0)
        {
	        return node->data;
        }
	    node = comp < 0 ? node->left : node->right;
    }
    node = malloc(sizeof *node);
    if (node == NULL)
    {
        return NULL;
    }
    node->data = data;
    node->left = node->right = rbnil(tree);
    node->parent = parent;
    if ((parent == rbroot(tree)) || (tree->comp(data, parent->data) < 0))
    {
	    parent->left = node;
    }
    else
    {
	    parent->right = node;
    }
    node->color = red;

    /*
     * If the parent node is black we are all set, if it is red we have
     * the following possible cases to deal with.  We iterate through
     * the rest of the tree to make sure none of the required properties
     * is violated.
     *
     *	1) The uncle is red.  We repaint both the parent and uncle black
     *     and repaint the grandparent node red.
     *
     *  2) The uncle is black and the new node is the right child of its
     *     parent, and the parent in turn is the left child of its parent.
     *     We do a left rotation to switch the roles of the parent and
     *     child, relying on further iterations to fixup the old parent.
     *
     *  3) The uncle is black and the new node is the left child of its
     *     parent, and the parent in turn is the left child of its parent.
     *     We switch the colors of the parent and grandparent and perform
     *     a right rotation around the grandparent.  This makes the former
     *     parent the parent of the new node and the former grandparent.
     *
     * Note that because we use a sentinel for the root node we never
     * need to worry about replacing the root.
     */
    while (node->parent->color == red)
    {
	    struct node *uncle;

	    if (node->parent == node->parent->parent->left)
        {
	        uncle = node->parent->parent->right;
	        if (uncle->color == red)
            {
		        node->parent->color = black;
		        uncle->color = black;
		        node->parent->parent->color = red;
		        node = node->parent->parent;
	        }
            else
            {
		        if (node == node->parent->right)
                {
		            node = node->parent;
		            rotate_left(tree, node);
		        }
		        node->parent->color = black;
		        node->parent->parent->color = red;
		        rotate_right(tree, node->parent->parent);
	        }
	    }
        else
        {
	        uncle = node->parent->parent->left;
	        if (uncle->color == red)
            {
		        node->parent->color = black;
		        uncle->color = black;
		        node->parent->parent->color = red;
		        node = node->parent->parent;
	        }
            else
            {
		        if (node == node->parent->left)
                {
		            node = node->parent;
		            rotate_right(tree, node);
		        }
		        node->parent->color = black;
		        node->parent->parent->color = red;
		        rotate_left(tree, node->parent->parent);
	        }
	    }
    }

    /* first node is always black */
    rbfirst(tree)->color = black;
    return data;
}

/*
 * Returns the successor of node, or nil if there is none.
 */
static struct node *successor(rbtree *tree, struct node *node)
{
    struct node *succ;

    if ((succ = node->right) != rbnil(tree))
    {
	    while (succ->left != rbnil(tree))
        {
	        succ = succ->left;
        }
    } else {
	    /* No right child, move up until we find it or hit the root */
	    for (succ = node->parent; node == succ->right; succ = succ->parent)
        {
	        node = succ;
        }
	    if (succ == rbroot(tree))
        {
	        succ = rbnil(tree);
        }
    }
    return succ;
}

/*
 * Repair the tree after a node has been deleted by rotating and repainting
 * colors to restore the 4 properties inherent in red-black trees.
 */
static void repair(rbtree *tree, struct node *node)
{
    struct node *sibling;

    while ((node->color == black) && (node != rbfirst(tree)))
    {
	    if (node == node->parent->left)
        {
	        sibling = node->parent->right;
	        if (sibling->color == red)
            {
		        sibling->color = black;
		        node->parent->color = red;
		        rotate_left(tree, node->parent);
		        sibling = node->parent->right;
	        }
	        if ((sibling->right->color == black) &&
                (sibling->left->color == black))
            {
		        sibling->color = red;
		        node = node->parent;
	        }
            else
            {
		        if (sibling->right->color == black)
                {
                    sibling->left->color = black;
                    sibling->color = red;
                    rotate_right(tree, sibling);
                    sibling = node->parent->right;
		        }
		        sibling->color = node->parent->color;
		        node->parent->color = black;
		        sibling->right->color = black;
		        rotate_left(tree, node->parent);
		        node = rbfirst(tree);
	        }
	    }
        else
        {
	        sibling = node->parent->left;
	        if (sibling->color == red)
            {
		        sibling->color = black;
		        node->parent->color = red;
		        rotate_right(tree, node->parent);
		        sibling = node->parent->left;
	        }
	        if ((sibling->right->color == black) &&
                (sibling->left->color == black))
            {
		        sibling->color = red;
		        node = node->parent;
	        }
            else
            {
		        if (sibling->left->color == black)
                {
		            sibling->right->color = black;
		            sibling->color = red;
		            rotate_left(tree, sibling);
		            sibling = node->parent->left;
		        }
		        sibling->color = node->parent->color;
		        node->parent->color = black;
		        sibling->left->color = black;
		        rotate_right(tree, node->parent);
		        node = rbfirst(tree);
	        }
	    }
    }
    node->color = black;
}

/*
 * Delete node 'z' from the tree and return its data pointer.
 */
void *rbtree_delete(rbtree *tree, const void *data)
{
    struct node *z = rbfirst(tree);

    while (z != rbnil(tree))
    {
        int comp;

	    if ((comp = tree->comp(data, z->data)) == 0)
        {
	        break;
        }
	    z = comp < 0 ? z->left : z->right;
    }
    if (z == rbnil(tree))
    {
        return NULL;
    }

    struct node *x, *y;

    if ((z->left == rbnil(tree)) || (z->right == rbnil(tree)))
    {
	    y = z;
    }
    else
    {
	    y = successor(tree, z);
    }
    x = (y->left == rbnil(tree)) ? y->right : y->left;
    if ((x->parent = y->parent) == rbroot(tree))
    {
	    rbfirst(tree) = x;
    }
    else
    {
	    if (y == y->parent->left)
        {
	        y->parent->left = x;
        }
	    else
        {
	        y->parent->right = x;
        }
    }
    if (y->color == black)
    {
	    repair(tree, x);
    }
    if (y != z)
    {
	    y->left = z->left;
	    y->right = z->right;
	    y->parent = z->parent;
	    y->color = z->color;
	    z->left->parent = z->right->parent = y;
	    if (z == z->parent->left)
        {
	        z->parent->left = y;
        }
	    else
        {
	        z->parent->right = y;
        }
    }

    void *temp = z->data;

    free(z); 
    return temp;
}

/*
 * Look for a node matching key in tree.
 * Returns a pointer to the node if found, else NULL.
 */
void *rbtree_search(rbtree *tree, const void *data)
{
    struct node *node = rbfirst(tree);

    while (node != rbnil(tree))
    {
        int comp;

	    if ((comp = tree->comp(data, node->data)) == 0)
        {
	        return node->data;
        }
	    node = comp < 0 ? node->left : node->right;
    }
    return NULL;
}

/*
 * Call func() for each node, passing it the node data and a cookie;
 * If func() returns non-zero for a node, the traversal stops and the
 * error value is returned.  Returns 0 on successful traversal.
 */
static void *walk(rbtree *tree, struct node *node,
    void *(*func)(void *, void *), void *cookie)
{
    if (node != rbnil(tree))
    {
        void *result;

	    if ((result = walk(tree, node->left, func, cookie)) != NULL)
        {
	        return result;
        }
        if ((result = func(node->data, cookie)) != NULL)
        {
		    return result;
        }
	    if ((result = walk(tree, node->right, func, cookie)) != NULL)
        {
	        return result;
        }
    }
    return NULL;
}

void *rbtree_walk(rbtree *tree, void *(*func)(void *, void *), void *cookie)
{
    return walk(tree, rbfirst(tree), func, cookie);
}

/*
 * Recursive portion of rbdestroy().
 */
static void destroy(rbtree *tree, struct node *node, void (*func)(void *))
{
    if (node != rbnil(tree))
    {
	    destroy(tree, node->left, func);
	    destroy(tree, node->right, func);
	    if (func != NULL)
        {
	        func(node->data);
        }
	    free(node);
    }
}

/*
 * Destroy the specified tree, calling the destructor destroy
 * for each node and then freeing the tree itself.
 */
void rbtree_destroy(rbtree *tree, void (*func)(void *))
{
    destroy(tree, rbfirst(tree), func);
    free(tree);
}

