/*
 * Test of Concurrency Kit, ck_queue
 * gcc
 */

#include <stdio.h>
#include <pthread.h>

#include <ck_ring.h>


int test_ring();

int main()
{
    test_ring();

    return 0;
}


/* The ring was initialized for 1023 elements. */
ck_ring_buffer_t buffer[102];
char tmp[] = "hello";

// for test_ring, arg to &myring
void* ring_producer(void *arg) {
    ck_ring_t* myring = (ck_ring_t*)arg;
    unsigned int ring_size = ck_ring_size(myring);
    unsigned int ring_capacity = ck_ring_capacity(myring);

    printf("myring size/capacity: %d/%d\n", ring_size, ring_capacity);

    // int x = 9;
    // void *result = tmp;
    unsigned int length = strlen(tmp);

    printf("producer running...\n");
    if (ck_ring_enqueue_mpmc_size(myring, buffer, tmp, &length) == false) {
        printf("produce msg failed\n");
    } else {
        printf("produce length %d, %p\n", length, tmp);
    }

    char r[32] = {0};
    if (ck_ring_dequeue_mpmc(myring, buffer, &r) == false) {
        printf("consume msg failed\n");
    } else {
        // char* x = (char*)r;
        printf("consume result %p, %s\n", r, r);
    }
    printf("producer end\n");
    return ((void*)0);
}

// for test_ring, arg to &myring
void* ring_consumer(void *arg) {
    ck_ring_t* myring = (ck_ring_t*)arg;
    void *result;

    printf("consumer running...\n");
    if (ck_ring_dequeue_mpmc(myring, buffer, &result) == false) {
        printf("consume msg failed\n");
    }
    int* x = (int*)result;
    printf("consumer end with result %p, %ls\n", x, x);
    return ((void*)0);
}

int test_ring() {

    ck_ring_t myring;
    ck_ring_init(&myring, 100);
    unsigned int ring_size = ck_ring_size(&myring);
    unsigned int ring_capacity = ck_ring_capacity(&myring);

    printf("myring size/capacity: %d/%d\n", ring_size, ring_capacity);

    int producer_num = 1;
    // int consumer_num = 3;
    pthread_t tid_producer[producer_num];
    // pthread_t tid_consumer[consumer_num];

    // for (int i=0; i<consumer_num; i++) {
    //     printf("consumer %d prepare to run\n", i);
    //     pthread_create(&tid_consumer[i], NULL, &ring_consumer, &myring);
    // }

    for (int i=0; i<producer_num; i++) {
        printf("producer %d prepare to run\n", i);
        pthread_create(&tid_producer[i], NULL, ring_producer, ((void*)&myring));
    }

    // #################################

    // block here to join threads
    for (int i=0; i<producer_num; i++) {
        pthread_join(tid_producer[i], NULL);
        printf("producer %d ended\n", i);
    }

    // for (int i=0; i<consumer_num; i++) {
    //     pthread_join(&tid_consumer[i], NULL);
    //     printf("consumer %d enged\n", i);
    // }

    return 0;
}


