#include <zmq.h>
#include <unistd.h>
#include <assert.h>
#include "inputer.h"


void *context;
void *puller;

int inputer_init(int port) {
    char addr[16] = {0};

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
    zmq_term (context);
    return 0;
}

void* inputer(void *arg) {
    int port = (*(int*)arg);
    int rc;

    printf("inputer listen port %d, start zmq loop\n", port);
    char buffer [100];
    while (1) {
        rc = zmq_recv (puller, buffer, sizeof(buffer), 0);
        if (rc == -1) {
            printf("zmq recv failed: %d, %s\n", zmq_errno(), zmq_strerror(zmq_errno()));
            break;
        }
        printf ("Received Hello\n");
        sleep (1);          //  Do some 'work'
        // zmq_send (responder, "World", 5, 0);
    }
    printf("END zmq loop\n");

    return ((void*)0);
}
