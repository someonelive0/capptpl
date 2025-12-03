/*
 * Test of lfqueue-1.2.2, that is a lockfree queue
 * gcc
 */

#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#include "lfqueue.h"


int test_queue();

int main()
{
    test_queue();

    return 0;
}


const int msg_num_pre_thread = 1024000;

// for test_queue, arg to &mqueue
void* queue_producer(void *arg) {
    printf("producer running...\n");

	lfqueue_t* myq = (lfqueue_t*)arg;

	int i = 0;
	int *int_data;
	while (i < msg_num_pre_thread) {
		int_data = (int*)malloc(sizeof(int));
		assert(int_data != NULL);
		*int_data = i++;
		/*Enqueue*/
		while (lfqueue_enq(myq, int_data)) {
			printf("ENQ FULL?\n");
		}

		/*Dequeue*/
		// while ((int_data = lfqueue_deq(myq)) == NULL) {
		// 	lfqueue_sleep(1);
		// }
		// printf("dequeue: %d\n", *int_data);
		// free(int_data);
	}

    printf("producer ending...\n");
    return ((void*)0);
}

// for test_queue, arg to &mqueue
void* queue_consumer(void *arg) {
    printf("consumer running...\n");

	lfqueue_t* myq = (lfqueue_t*)arg;

	int i = 0;
	int *int_data;
	while (i < msg_num_pre_thread) {
		/*Dequeue*/
		if ((int_data = lfqueue_deq(myq)) == NULL) {
			lfqueue_sleep(1);
			continue;
		}
		// printf("dequeue: %d\n", *int_data);
		free(int_data);
		i++;
	}

     printf("consumer ending...\n");
    return ((void*)0);
}

int test_queue() {

    lfqueue_t myqueue;
	if (lfqueue_init(&myqueue) == -1) {
		printf ("init queue failed\n");
		return -1;
	}

	size_t qsize = lfqueue_size(&myqueue);
    printf("myqueue size: %ld\n", qsize);

    int producer_num = 3, consumer_num = 3;
    pthread_t tid_producer[producer_num], tid_consumer[consumer_num];

    for (int i=0; i<consumer_num; i++) {
        printf("consumer %d prepare to run\n", i);
        pthread_create(&tid_consumer[i], NULL, &queue_consumer, &myqueue);
    }

    for (int i=0; i<producer_num; i++) {
        printf("producer %d prepare to run\n", i);
        pthread_create(&tid_producer[i], NULL, queue_producer, ((void*)&myqueue));
    }

    // #################################

    // block here to join threads
    for (int i=0; i<producer_num; i++) {
        pthread_join(tid_producer[i], NULL);
        printf("producer %d ended\n", i);
    }

    for (int i=0; i<consumer_num; i++) {
        pthread_join(tid_consumer[i], NULL);
        printf("consumer %d enged\n", i);
    }

	qsize = lfqueue_size(&myqueue);
    printf("myqueue left size should be zero: %ld\n", qsize);
	assert(0 == qsize);
	
    return 0;
}
