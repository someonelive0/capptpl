/*
 *  gcc -I../lib -I../lib/uthash-2.3.0 testpool.c
 */
#include <stdio.h>

#include "utringbuffer.h"


/*
 * packet memory pool
 */
struct packet_pool {
    int           size;
    void*         readp;
    void*         writep;
    UT_ringbuffer *ringbuffer;
};

struct mydata {
    int    a;
    char*  s;
};


struct packet_pool* packet_pool_new(int size);
int packet_pool_free(struct packet_pool* pool);



UT_icd addr_icd = {sizeof(void*), NULL, NULL, NULL };

struct packet_pool* packet_pool_new(int size)
{
    struct packet_pool* pool = malloc(sizeof(struct packet_pool));
    memset(pool, 0, sizeof(struct packet_pool));
    pool->size = size;

    utringbuffer_new(pool->ringbuffer, size, &addr_icd);
    if (NULL == pool->ringbuffer) goto err;

    struct mydata* pkt = NULL;
    for (int i=0; i<size; i++) {
        pkt = malloc(sizeof(struct mydata));
        memset(pkt, 0, sizeof(struct mydata));
        pkt->a = i;
        // pkt->s = sdsempty();

        printf("push %d, addr %p\n", pkt->a, pkt);
        utringbuffer_push_back(pool->ringbuffer, &pkt);
    }
    pool->writep = &pkt;

    return pool;

err:
    free(pool);
    return NULL;
}

int packet_pool_free(struct packet_pool* pool)
{
    struct mydata **ppkt = NULL;
    struct mydata *pkt = NULL;
    if (pool) {
        while ( (ppkt=(struct mydata**)utringbuffer_next(pool->ringbuffer, ppkt))) {
            pkt = *ppkt;
            if (pkt->s) {
                // sdsfree(pkt->data);
                pkt->s = NULL;
            }
            printf("free int %d, addr %p\n", pkt->a, pkt);
            free(pkt);
        }
    }

    utringbuffer_free(pool->ringbuffer);
    pool->ringbuffer = NULL;

    return 0;
}


int main()
{
    struct packet_pool *pkt_pool = packet_pool_new(10);
    if (NULL == pkt_pool) {
        printf("new pool failed\n");
        exit(1);
    }
    printf("new pool size %d\n", utringbuffer_len(pkt_pool->ringbuffer));
    printf("new full %d\n", utringbuffer_full(pkt_pool->ringbuffer));

    struct mydata **ppkt = NULL;
    struct mydata *pkt = NULL;
    while ( (ppkt=(struct mydata**)utringbuffer_next(pkt_pool->ringbuffer, ppkt))) {
        pkt = *ppkt;
        printf("next int %d, addr %p\n", pkt->a, pkt);
    }
    printf("after pool size %d\n", utringbuffer_len(pkt_pool->ringbuffer));


    packet_pool_free(pkt_pool);

    exit(0);
}
