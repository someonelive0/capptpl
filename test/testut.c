#include <stdio.h>

#include "utringbuffer.h"
#include "uthash.h"
#include "utarray.h"


int test_ringbuffer_int();
int test_ringbuffer_struct();
int test_hash_str();
int test_vector_struct();

int main()
{
    test_ringbuffer_int();
    test_ringbuffer_struct();
    test_hash_str();
    printf ("1------>\n");
    test_vector_struct();
    return 0;
}

int test_ringbuffer_int()
{
    UT_ringbuffer *history;
    int i, *p;

    printf("* UT_ringbuffer of int\n");

    utringbuffer_new(history, 7, &ut_int_icd);
    for (i=0; i < 10; i++) utringbuffer_push_back(history, &i);

    for (p = (int*)utringbuffer_front(history);
         p != NULL;
         p = (int*)utringbuffer_next(history, p)) {
        printf("%d\n", *p);  /* prints "3 4 5 6 7 8 9" */
    }
    printf("\n");

    for (i=0; i < utringbuffer_len(history); i++) {
        p = utringbuffer_eltptr(history, i);
        printf("%d\n", *p);  /* prints "3 4 5 6 7 8 9" */
    }

    utringbuffer_free(history);
    putchar('\n');
    return 0;
}


/*
 * ringbuffer_struct
 */
typedef struct {
    int a;
    char *s;
} intchar_t;

void intchar_copy(void *_dst, const void *_src) {
  intchar_t *dst = (intchar_t*)_dst, *src = (intchar_t*)_src;
  dst->a = src->a;
  dst->s = src->s ? strdup(src->s) : NULL;
}

void intchar_dtor(void *_elt) {
  intchar_t *elt = (intchar_t*)_elt;
  free(elt->s);
}

UT_icd intchar_icd = {sizeof(intchar_t), NULL, intchar_copy, intchar_dtor};

int test_ringbuffer_struct()
{
    UT_ringbuffer *intchars;
    intchar_t ic, *p;
    utringbuffer_new(intchars, 2, &intchar_icd);

    printf("* UT_ringbuffer of struct\n");

    ic.a=1; ic.s="hello"; utringbuffer_push_back(intchars, &ic);
    ic.a=2; ic.s="world"; utringbuffer_push_back(intchars, &ic);
    ic.a=3; ic.s="peace"; utringbuffer_push_back(intchars, &ic);

    p = NULL;
    while ( (p=(intchar_t*)utringbuffer_next(intchars,p))) {
        printf("%d %s\n", p->a, (p->s ? p->s : "null"));
        /* prints "2 world 3 peace" */
    }

    utringbuffer_free(intchars);
    putchar('\n');
    return 0;
}

/*
 * hash_str
 */
struct my_struct {
    char name[10];             /* key (string is WITHIN the structure) */
    int id;
    UT_hash_handle hh1;        /* handle for first hash table */
    UT_hash_handle hh2;        /* handle for second hash table */
    // UT_hash_handle hh;         /* makes this structure hashable */
};

int test_hash_str()
{
    const char *names[] = { "joe", "bob", "betty", NULL };
    struct my_struct *s, *tmp, *users_by_id = NULL, *users_by_name = NULL;

    printf("* uthash of string\n");

    for (int i = 0; names[i]; ++i) {
        s = (struct my_struct *)malloc(sizeof *s);
        strcpy(s->name, names[i]);
        s->id = i;
        /* add the structure to both hash tables */
        HASH_ADD(hh1, users_by_id, id, sizeof(int), s);
        HASH_ADD(hh2, users_by_name, name, strlen(s->name), s);
        // HASH_ADD_STR(users, name, s);
    }

    /* find user by ID in the "users_by_id" hash table */
    int i = 1;
    HASH_FIND(hh1, users_by_id, &i, sizeof(int), s);
    if (s) printf("found id %d: %s\n", i, s->name);

    /* find user by name in the "users_by_name" hash table */
    char *name = "betty";
    HASH_FIND(hh2, users_by_name, name, strlen(name), s);
    if (s) printf("found user %s: %d\n", name, s->id);

    // HASH_FIND_STR(users, "betty", s);
    // if (s) printf("betty's id is %d\n", s->id);

    /* free the hash table contents */
    HASH_ITER(hh1, users_by_id, s, tmp) {
        HASH_DELETE(hh1, users_by_id, s);
        // HASH_DEL(users, s);
        // free(s); // don't free it, will cause coredump, because hh2 will free it
    }
    HASH_ITER(hh2, users_by_name, s, tmp) {
        HASH_DELETE(hh2, users_by_name, s);
        free(s); // repeat free, maybe a bug.
    }
    putchar('\n');
    return 0;
}

int test_vector_struct()
{
    UT_array *intchars;
    intchar_t ic, *p;
    utarray_new(intchars, &intchar_icd);

    printf("* utvector of struct\n");

    ic.a=11; ic.s="hello hello"; utarray_push_back(intchars, &ic);
    ic.a=22; ic.s="world world"; utarray_push_back(intchars, &ic);

    p=NULL;
    while ( (p=(intchar_t*)utarray_next(intchars,p))) {
        printf("%d %s\n", p->a, (p->s ? p->s : "null"));
    }

    utarray_free(intchars);
    putchar('\n');
    return 0;
}
