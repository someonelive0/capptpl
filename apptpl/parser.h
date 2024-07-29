#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "cchan_pthread.h"

#define PARSER_WAIT_MS 200


struct parser {
    int     shutdown;
    cchan_t *chan_pkt;
    uint64_t count;     // total packet number
    uint64_t bytes;    // total byte number;
    int      timer_interval; // when timer to call this. seconds
};

// arg is struct parser*
void* parser_loop(void *arg);
void parser_time_ev(struct parser* prsr, int seconds);

#endif // PARSER_H
