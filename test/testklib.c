#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "kavl.h"
#include "kbtree.h"


int test_avl(void);
int test_btree(void);

int main(void)
{
    test_avl();
    test_btree();
    return 0;
}

/*
 * avl sort
 */
struct my_node {
    char key;
    KAVL_HEAD(struct my_node) head;
};
#define my_cmp(p, q) (((q)->key < (p)->key) - ((p)->key < (q)->key))
KAVL_INIT(my, struct my_node, head, my_cmp)

int test_avl(void)
{
    const char *str = "MNOLKQOPHIA"; // from wiki, except a duplicate
    struct my_node *root = 0;
    int i, l = strlen(str);

    printf("* avl to sort string %s\n", str);

    for (i = 0; i < l; ++i) {        // insert in the input order
        struct my_node *q, *p = malloc(sizeof(*p));
        p->key = str[i];
        q = kavl_insert(my, &root, p, 0);
        if (p != q) free(p);         // if already present, free
    }
    kavl_itr_t(my) itr;
    kavl_itr_first(my, root, &itr);  // place at first
    do {                             // traverse
        const struct my_node *p = kavl_at(&itr);
        putchar(p->key);
        free((void*)p);              // free node
    } while (kavl_itr_next(my, &itr));
    putchar('\n');
    putchar('\n');
    return 0;
}


/*
 * btree sort
 */
typedef struct {
    char *key;
    int count;
} elem_t;

#define elem_cmp(a, b) (strcmp((a).key, (b).key))
KBTREE_INIT(str, elem_t, elem_cmp)

int test_btree(void)
{
    kbtree_t(str) *b;
    elem_t *p, t;
    kbitr_t itr;
    int i;
    const char *names[] = { "joe", "bob", "bob", "betty", "tom", "jerry", "tom", NULL };

    b = kb_init(str, KB_DEFAULT_SIZE);

    printf("* btree to sort string vector %s\n", *names);

    for (i = 0; names[i]; ++i) {
        // no need to allocate; just use pointer
        t.key = (char *)names[i], t.count = 1;
        p = kb_getp(str, b, &t); // kb_get() also works
        // IMPORTANT: put() only works if key is absent
        if (!p) kb_putp(str, b, &t);
        else ++p->count;
    }

    // ordered tree traversal
    kb_itr_first(str, b, &itr); // get an iterator pointing to the first
    for (; kb_itr_valid(&itr); kb_itr_next(str, b, &itr)) { // move on
        p = &kb_itr_key(elem_t, &itr);
        printf("%d\t%s\n", p->count, p->key);
    }
    kb_destroy(str, b);

    return 0;
}
