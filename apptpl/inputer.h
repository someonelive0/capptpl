#ifndef INPUTER_H
#define INPUTER_H

#include "cchan_pthread.h"


struct inputer {
    int  port;
    cchan_t *chan_msg;
    void *zmq_context;
    void *puller;
};

int inputer_open(struct inputer* inptr, int port, cchan_t *chan_msg);
int inputer_stop(struct inputer* inptr);
// arg is struct inputer*
void* inputer_loop(void *arg);

#endif // INPUTER_H
