#include "utringbuffer.h"



/*
 * packet memory pool
 */
struct packet_pool {
    int           size;
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
    pool->size = size;
    pool->ringbuffer = NULL;
    utringbuffer_new(pool->ringbuffer, size, &addr_icd);
    if (NULL == pool->ringbuffer) goto err;

    for (int i=0; i<size; i++) {
        struct mydata* pkt = malloc(sizeof(struct mydata));
        memset(pkt, 0, sizeof(struct mydata));
        pkt->data = sdsempty();
        utringbuffer_push_back(pool->ringbuffer, &pkt);
    }

    return pool;

err:
    free(pool);
    return NULL;
}

int packet_pool_free(struct packet_pool* pool)
{
    struct mydata *pkt = NULL;
    if (pool) {
        while ( (pkt=(struct mydata*)utringbuffer_next(pool->ringbuffer, pkt))) {
            if (pkt->data) {
                sdsfree(pkt->data);
                pkt->data = NULL;
            }
            free(pkt);
        }
    }

    return 0;
}