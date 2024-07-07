#include <zmq.h>
#include <unistd.h>
#include <stdlib.h>

#include "cchan_pthread.h"

#include <assert.h>
#include "inputer.h"


void *context;
void *puller;
int puller_port;

int inputer_init(int port) {
    char addr[16] = {0};
    puller_port = port;

    context = zmq_ctx_new();
    puller = zmq_socket (context, ZMQ_PULL);
    snprintf(addr, sizeof(addr)-1, "tcp://*:%d", port);
    int rc = zmq_bind (puller, addr);
    if (rc == -1) {
        printf("zmq zmq_bind port %d failed: %d, %s\n", port, zmq_errno(), zmq_strerror(zmq_errno()));
        return -1;
    }
    return 0;
}

int inputer_stop() {
    zmq_close (puller);
    zmq_ctx_term (context);
    return 0;
}

void* inputer(void *arg) {
    cchan_t *chan_msg = ((cchan_t*)arg);
    int rc;

    printf("inputer listen port %d, start zmq loop, zmq_msg size %lld\n",
        puller_port, sizeof(zmq_msg_t));
    // char buffer [100];
    zmq_msg_t *msg = NULL;
    while (1) {
        if (NULL == (msg = (zmq_msg_t *)malloc(sizeof(zmq_msg_t)))) {
            printf("malloc zmq_msg_t failed\n");
            break;
        }
        if ((rc = zmq_msg_init (msg)) != 0) {
            printf("zmq zmq_msg_init failed: %d, %s\n", zmq_errno(), zmq_strerror(zmq_errno()));
            continue;
        }
        
        // rc = zmq_recv (puller, buffer, sizeof(buffer), 0);
        rc = zmq_msg_recv (msg, puller, 0);
        if (rc == -1) {
            printf("zmq recv failed: %d, %s\n", zmq_errno(), zmq_strerror(zmq_errno()));
            break;
        }
        // printf("inputer rcv msg %lld: [%s]\n", zmq_msg_size (msg), (char*)zmq_msg_data (msg));

        cchan_send(chan_msg, &msg);

        // to limit channel of msg size
        if (chan_msg->used >= 16384) {
            //printf("chan_msg used size: %d, %d\n", chan_msg->used, chan_msg->size);
            usleep(100);
        }
    }
    printf("END zmq loop\n");

    return ((void*)0);
}
