#include <zmq.h>
#include <stdlib.h>
#include <unistd.h>

#include "logger.h"

#include "worker.h"


// arg is struct worker*
void* worker_loop(void *arg)
{
    int rc;
    zmq_msg_t *msg = NULL;
    uint64_t count = 0;

    struct worker* wrkr = arg;

    LOG_INFO ("worker loop");
    wrkr->shutdown = 0;
    while (!wrkr->shutdown) {
        if (0 == cchan_waittime(wrkr->chan_msg, &msg, WORKER_WAIT_MS)) { // 0 means timeout
            // printf("cchan_waittime\n");
            //if (chan_msg->used > 10000) {
            // printf("chan_msg used, alloc: %d, %d\n", chan_msg->used, chan_msg->alloc);
            //}
            continue;
        }

        // LOG_TRACE ("worker rcv msg %lld: [%s]\n", zmq_msg_size (msg), (char*)zmq_msg_data (msg));
        count ++;
        if ((count % 10000) == 0) {
            LOG_DEBUG ("worker rcv count times %zu, len %zu: [%s]",
                       count, zmq_msg_size (msg), (char*)zmq_msg_data (msg));
        }

        if ((rc = zmq_msg_close(msg)) != 0) {
            LOG_ERROR ("zmq_msg_close msg failed: %d, %s", zmq_errno(), zmq_strerror(zmq_errno()));
        }
        free(msg);
    }
    LOG_INFO ("END worker loop");

    return ((void*)0);
}

#if 0
int worker_shutdown;

void* worker(void *arg)
{
    cchan_t *chan_msg = ((cchan_t*)arg);
    int rc;
    zmq_msg_t *msg = NULL;
    int wait_ms = 200;
    int count = 0;

    LOG_INFO ("worker loop");
    worker_shutdown = 0;
    while (!worker_shutdown) {
        if (0 == cchan_waittime(chan_msg, &msg, wait_ms)) { // 0 means timeout
            // printf("cchan_waittime\n");
            //if (chan_msg->used > 10000) {
            // printf("chan_msg used, alloc: %d, %d\n", chan_msg->used, chan_msg->alloc);
            //}
            continue;
        }

        // LOG_TRACE ("worker rcv msg %lld: [%s]\n", zmq_msg_size (msg), (char*)zmq_msg_data (msg));
        count ++;
        if ((count % 10000) == 0) {
            LOG_DEBUG ("worker rcv count times %d, len %lld: [%s]",
                       count, zmq_msg_size (msg), (char*)zmq_msg_data (msg));
        }

        if ((rc = zmq_msg_close(msg)) != 0) {
            LOG_ERROR ("zmq_msg_close msg failed: %d, %s", zmq_errno(), zmq_strerror(zmq_errno()));
        }
        free(msg);
    }
    LOG_INFO ("END worker loop");

    return ((void*)0);
}
#endif
