#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "cchan_pthread.h"

#include "word_policy.h"
#include "re_policy.h"

#define PARSER_WAIT_MS 200


struct parser {
    int     shutdown;
    cchan_t *chan_pkt;
    uint64_t count;     // total packet number
    uint64_t bytes;    // total byte number;
    uint64_t word_match_count;     // total word policy matched number
    int      timer_interval; // when timer to call this. seconds

    struct word_policy wordp;
    struct re_policy   rep;
};

int parser_create(struct parser* prsr, cchan_t *chan_pkt, 
                const char* word_file, const char* regex_file);
int parser_destroy(struct parser* prsr);
// arg is struct parser*
void* parser_loop(void *arg);
void parser_time_ev(struct parser* prsr, int seconds);

#endif // PARSER_H
