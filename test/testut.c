#include <stdio.h>
#include "utringbuffer.h"


int test_ringbuffer_int();
int test_ringbuffer_struct();

int main()
{
    test_ringbuffer_int();
    test_ringbuffer_struct();
    return 0;
}

int test_ringbuffer_int()
{
    UT_ringbuffer *history;
    int i, *p;

    printf("UT_ringbuffer of int\n");

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

    return 0;
}

typedef struct {
    int a;
    int b;
} intpair_t;

UT_icd intpair_icd = {sizeof(intpair_t), NULL, NULL, NULL};

int test_ringbuffer_struct()
{
    UT_ringbuffer *pairs;
    intpair_t ip, *p;

    printf("\nUT_ringbuffer of struct\n");

    utringbuffer_new(pairs, 7, &intpair_icd);

    ip.a=1;
    ip.b=2;
    utringbuffer_push_back(pairs, &ip);
    ip.a=10;
    ip.b=20;
    utringbuffer_push_back(pairs, &ip);

    for (p=(intpair_t*)utringbuffer_front(pairs);
         p!=NULL;
         p=(intpair_t*)utringbuffer_next(pairs,p)) {
        printf("%d, %d\n", p->a, p->b);
    }

    utringbuffer_free(pairs);
    return 0;
}
