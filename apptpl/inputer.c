#include <zmq.h>
#include <unistd.h>
#include <stdlib.h>

#include "logger.h"

#include "inputer.h"


int inputer_open(struct inputer* inptr, int port, cchan_t *chan_msg)
{
    char addr[16] = {0};
    int major, minor, patch = 0;
    void *zmq_context = NULL;
    void *puller = NULL;

    if (inptr->puller) {
        LOG_ERROR ("inputer had opened");
        return -1;
    }

    zmq_version(&major, &minor, &patch);
    LOG_DEBUG ("zeromq version: %d.%d.%d", major, minor, patch);

    if (NULL == (zmq_context = zmq_ctx_new())) {
        LOG_ERROR ("zmq_ctx_new failed");
        return -1;
    }
    if (NULL == (puller = zmq_socket (zmq_context, ZMQ_PULL))) {
        LOG_ERROR ("zmq_socket ZMQ_PULL failed");
        goto err;
    }

    snprintf(addr, sizeof(addr)-1, "tcp://*:%d", port);
    if (zmq_bind (puller, addr) == -1) {
        LOG_ERROR ("zmq zmq_bind port %d failed: %d, %s",
                   port, zmq_errno(), zmq_strerror(zmq_errno()));
        goto err;
    }
    goto done;

err:
    if (puller) zmq_close (puller);
    if (zmq_context) zmq_ctx_term (zmq_context);
    return -1;

done:
    inptr->port = port;
    inptr->chan_msg = chan_msg;
    inptr->zmq_context = zmq_context;
    inptr->puller = puller;
    inptr->count = 0;
    return 0;
}

int inputer_stop(struct inputer* inptr)
{
    if (inptr->puller) {
        zmq_close (inptr->puller);
        inptr->puller = NULL;
    }
    if (inptr->zmq_context) {
        zmq_ctx_term (inptr->zmq_context);
        inptr->zmq_context = NULL;
    }
    return 0;
}

// arg is struct inputer*
void* inputer_loop(void *arg)
{
    int rc;
    // char buffer [100];

    struct inputer* inptr = arg;
    LOG_INFO ("inputer listen port %d, start zmq loop, zmq_msg_t size %zu",
              inptr->port, sizeof(zmq_msg_t));

    zmq_msg_t *msg = NULL;
    while (1) {
        if (NULL == (msg = (zmq_msg_t *)malloc(sizeof(zmq_msg_t)))) {
            LOG_ERROR ("inputer malloc zmq_msg_t failed, break");
            break;
        }
        if ((rc = zmq_msg_init (msg)) != 0) {
            LOG_WARN ("inputer zmq_msg_init failed: %d, %s", zmq_errno(), zmq_strerror(zmq_errno()));
            free(msg);
            continue;
        }

        // rc = zmq_recv (puller, buffer, sizeof(buffer), 0);
        rc = zmq_msg_recv (msg, inptr->puller, 0);
        if (rc == -1) {
            if (ETERM == zmq_errno()) // ETERM = 156384765, Context was terminated
                LOG_INFO ("inputer recv ETERM(%d), %s", zmq_errno(), zmq_strerror(zmq_errno()));
            else
                LOG_ERROR ("inputer recv failed: %d, %s", zmq_errno(), zmq_strerror(zmq_errno()));
            free(msg);
            break;
        }
        // LOG_DEBUG ("inputer rcv msg %lld: [%s]\n", zmq_msg_size (msg), (char*)zmq_msg_data (msg));

        inptr->count ++;
        cchan_send(inptr->chan_msg, &msg);

        // to limit channel of msg size
        if (inptr->chan_msg->used >= 16384) {
            // LOG_TRACE ("chan_msg used size: %d, %d\n", chan_msg->used, chan_msg->size);
            usleep(100);
        }
    }
    LOG_INFO ("END inputer loop, inputer count %zu", inptr->count);

    return ((void*)0);
}

#if 0
static void *zmq_context;
static void *puller;
static int puller_port;

int inputer_init(int port)
{
    char addr[16] = {0};
    int major, minor, patch = 0;
    puller_port = port;

    zmq_version(&major, &minor, &patch);
    LOG_DEBUG ("zeromq version: %d.%d.%d", major, minor, patch);

    zmq_context = zmq_ctx_new();
    puller = zmq_socket (zmq_context, ZMQ_PULL);
    snprintf(addr, sizeof(addr)-1, "tcp://*:%d", port);
    if (zmq_bind (puller, addr) == -1) {
        LOG_ERROR ("zmq zmq_bind port %d failed: %d, %s",
                   port, zmq_errno(), zmq_strerror(zmq_errno()));
        return -1;
    }
    return 0;
}

int inputer_stop()
{
    zmq_close (puller);
    zmq_ctx_term (zmq_context);
    return 0;
}

/*
 * loop to input zeromq messages, can be called by pthread
 */
void* inputer(void *arg)
{
    cchan_t *chan_msg = ((cchan_t*)arg);
    int rc;

    LOG_INFO ("inputer listen port %d, start zmq loop, zmq_msg_t size %lld",
              puller_port, sizeof(zmq_msg_t));
    // char buffer [100];
    zmq_msg_t *msg = NULL;
    while (1) {
        if (NULL == (msg = (zmq_msg_t *)malloc(sizeof(zmq_msg_t)))) {
            LOG_ERROR ("malloc zmq_msg_t failed, break");
            break;
        }
        if ((rc = zmq_msg_init (msg)) != 0) {
            LOG_WARN ("zmq zmq_msg_init failed: %d, %s", zmq_errno(), zmq_strerror(zmq_errno()));
            free(msg);
            continue;
        }

        // rc = zmq_recv (puller, buffer, sizeof(buffer), 0);
        rc = zmq_msg_recv (msg, puller, 0);
        if (rc == -1) {
            if (ETERM == zmq_errno()) // ETERM = 156384765, Context was terminated
                LOG_INFO ("zmq recv ETERM(%d), %s", zmq_errno(), zmq_strerror(zmq_errno()));
            else
                LOG_ERROR ("zmq recv failed: %d, %s", zmq_errno(), zmq_strerror(zmq_errno()));
            free(msg);
            break;
        }
        // LOG_DEBUG ("inputer rcv msg %lld: [%s]\n", zmq_msg_size (msg), (char*)zmq_msg_data (msg));

        cchan_send(chan_msg, &msg);

        // to limit channel of msg size
        if (chan_msg->used >= 16384) {
            // LOG_TRACE ("chan_msg used size: %d, %d\n", chan_msg->used, chan_msg->size);
            usleep(100);
        }
    }
    LOG_INFO ("END zmq loop");

    return ((void*)0);
}
#endif
