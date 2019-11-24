/*
This is some example code to get you started with rb3ptr. We're only covering
the basics here, not really exercising rb3ptr's flexibility.

To build, change to the directory containing these files (rb3ptr.c, rb3ptr.h,
examples.c) and run

    gcc -Wall -I. rb3ptr.c example.c -o test

To see more of rb3ptr's API, look at rb3ptr.h, which includes some
documentation.

For a better (but more involved) example what you can do with rb3ptr, check
out the textrope implementation in my project "astedit" at
http://jstimpfle.de/projects/astedit/astedit.html. Since the nodes in that
textrope don't even have a natural ordering, that project uses the low-level
primitives to define custom iteration functions.
 */

#include <stddef.h>  // offsetof()
#include <stdio.h>
#include <stdlib.h>
#include <rb3ptr.h>

enum {
        NUM_FOOS = 1024
};

/* Declare a container data structure, carrying some data, and including a
 * `struct rb3_head` link structure. The link structure will be used to link the
 * node in a tree.
*/

struct Foo {
        struct rb3_head head;
        int val;
};

/* Define how to get from an embedded link structure to the embedding struct Foo
 *
 * You could simply use a macro like container_of() which is used in the Linux
 * kernel, but I think it's nonstandard. So I'm not using it in this file.
 */

static struct Foo *get_foo(struct rb3_head *head)
{
        return (struct Foo *)((char *) head - offsetof(struct Foo, head));
}

/*
 * Define a helper function to drive iteration down a tree. With this you can
 * use high-level functions like rb3_insert() and rb3_delete(), to avoid having
 * to write custom iteration code. (But you can still do that of course)
 *
 * The functions that you can hand to rb3_insert() or rb3_delete() will often
 * just compare two nodes, but in general they take just a single struct
 * rb3_head and an additional context parameter. You can see in main() that
 * we've put a struct Foo as an argument to rb3_insert() and rb3_delete().
 * That's why we can cast the context parameter back to that struct Foo here.
 */

static int compare_foo_heads(struct rb3_head *a, void *data)
{
        struct Foo *x = get_foo(a);
        struct Foo *y = data;
        if (x->val > y->val)
                return 1;
        else if (x->val < y->val)
                return -1;
        return 0;
}

/*
 * Test insertions and deletions.
 */

int main(void)
{
        struct rb3_tree tree;
        struct rb3_head *iter;
        struct Foo *foo;
        size_t i;

        rb3_reset_tree(&tree);
        foo = malloc(NUM_FOOS * sizeof (struct Foo));

        /* make up some random values for the nodes. */
        for (i = 0; i < NUM_FOOS; i++)
                foo[i].val = rand();

        /* insert the random nodes. */
        for (i = 0; i < NUM_FOOS; i++)
                rb3_insert(&tree, &foo[i].head, compare_foo_heads, &foo[i]);

        /* Iterate over the tree using in-order traversal. We expect to print
         * a sorted sequence here due to the way we defined the ordering
         * function that we gave to rb3_insert(). */
        for (iter = rb3_get_min(&tree);
             iter != NULL;
             iter = rb3_get_next(iter))
                printf("iter %d\n", get_foo(iter)->val);

        /* Remove all the nodes from the tree. To guarantee that each node is
         * found we should use the same (or at least a compatible) ordering
         * function. */
        for (i = 0; i < NUM_FOOS; i++)
                rb3_delete(&tree, compare_foo_heads, &foo[i].head);

        free(foo);

        return 0;
}
