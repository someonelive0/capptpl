#ifndef WORKER_H
#define WORKER_H

#include "cchan_pthread.h"

#define WORKER_WAIT_MS 200


struct worker {
    int  shutdown;
    cchan_t *chan_msg;
};

// arg is struct worker*
void* worker_loop(void *arg);

#endif // WORKER_H
