#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdatomic.h>

#include "cchan_pthread.h"
#include "sds.h"

#include "word_policy.h"
#include "re_policy.h"

#define PARSER_WAIT_MS 200


struct parser {
    int           shutdown;
    int           timer_interval; // when timer to call this. seconds
    cchan_t*      chan_pkt;
    atomic_ullong count;     // total packet number
    atomic_ullong bytes;    // total byte number;
    atomic_ullong word_match_count;     // total word policy matched number
    atomic_ullong regex_match_count;     // total regex policy matched number

    struct word_policy wordp;
    struct re_policy   rep;
};

int parser_create(struct parser* prsr, cchan_t *chan_pkt, 
                const char* word_file, const char* regex_file);
int parser_destroy(struct parser* prsr);
// arg is struct parser*
void* parser_loop(void *arg);
void parser_time_ev(struct parser* prsr, int seconds);

// dump parser to json string, remember to free sds
sds parser_dump(const struct parser* prsr);

#endif // PARSER_H
