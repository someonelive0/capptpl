#include <zmq.h>
#include <stdlib.h>
#include <unistd.h>

#include "cchan_pthread.h"


int worker_shutdown;

void* worker(void *arg) {
    cchan_t *chan_msg = ((cchan_t*)arg);
    int rc;
    zmq_msg_t *msg = NULL;
    int wait_ms = 200;
    int count = 0;

    printf("worker loop\n");
    worker_shutdown = 0;
    while (!worker_shutdown) {
        if (0 == cchan_waittime(chan_msg, &msg, wait_ms)) { // 0 means timeout
            // printf("cchan_waittime\n");
            //if (chan_msg->used > 10000) {
                // printf("chan_msg used, alloc: %d, %d\n", chan_msg->used, chan_msg->alloc);
            //}
            continue;
        }

        // printf("worker rcv msg %lld: [%s]\n", zmq_msg_size (msg), (char*)zmq_msg_data (msg));
        count ++;
        if ((count % 10000) == 0) {
            printf("worker rcv count times %d, len %lld: [%s]\n",
                count, zmq_msg_size (msg), (char*)zmq_msg_data (msg));
        }

        if ((rc = zmq_msg_close(msg)) != 0) {
            printf("zmq_msg_close msg failed: %d, %s\n", zmq_errno(), zmq_strerror(zmq_errno()));
        }
        free(msg);
    }
    printf("END worker loop\n");

    return ((void*)0);
}
