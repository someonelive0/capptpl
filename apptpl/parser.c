#include "parser.h"

#include <stdlib.h>

#include "logger.h"

#include "pkt.h"


// thread var
static __thread struct parser* prsr;

// word match call back
static int match_word_cb(int strnum, int textpos, MEMREF const *pattv);


int parser_create(struct parser* prsr, cchan_t *chan_pkt, 
                const char* word_file, const char* regex_file)
{
    memset(&prsr->wordp, 0, sizeof(struct word_policy));
    prsr->chan_pkt = chan_pkt;
    if (-1 == word_policy_create(&prsr->wordp, word_file)) {
        return -1;
    }
    prsr->wordp.match_cb = match_word_cb;

    if (logger_getLevel() <= LogLevel_DEBUG)
        word_policy_dump(&prsr->wordp);

    if (-1 == re_policy_create(&prsr->rep, regex_file)) {
        word_policy_destroy(&prsr->wordp);
        return -1;
    }
    
    if (logger_getLevel() <= LogLevel_DEBUG)
        re_policy_dump(&prsr->rep);

    return 0;
}

int parser_destroy(struct parser* prsr)
{
    prsr->shutdown = 1;
    word_policy_destroy(&prsr->wordp);
    re_policy_destroy(&prsr->rep);

    return 0;
}

// arg is struct parser*, store it in thread var prsr.
void* parser_loop(void *arg)
{
    struct packet *pkt = NULL;
    int rc = 0;

    // struct parser* prsr = arg;
    prsr = arg; // use thread local var
    prsr->shutdown = 0;
    prsr->count = ATOMIC_VAR_INIT(0);
    prsr->bytes = ATOMIC_VAR_INIT(0);
    prsr->word_match_count = ATOMIC_VAR_INIT(0);
    prsr->regex_match_count = ATOMIC_VAR_INIT(0);
    int re_match[prsr->rep.rule_num];

    LOG_INFO ("parser loop");
    while (!prsr->shutdown) {
        // process timer_seconds first
        if (prsr->timer_interval) {
            LOG_DEBUG ("parser timer interval seconds %d, rcv count %zu, bytes %zu",
                        prsr->timer_interval,
                        atomic_load(&prsr->count), atomic_load(&prsr->bytes));
            prsr->timer_interval = 0;
        }

        if (0 == cchan_waittime(prsr->chan_pkt, &pkt, PARSER_WAIT_MS)) { // 0 means timeout
            // printf("cchan_waittime\n");
            //if (chan_pkt->used > 10000) {
            // printf("chan_pkt used, alloc: %d, %d\n", chan_pkt->used, chan_pkt->alloc);
            //}
            continue;
        }

        atomic_fetch_add(&prsr->count, 1);
        atomic_fetch_add(&prsr->bytes, pkt->hdr.caplen);
        // if ((atomic_load(&prsr->count) % 10000) == 0) {
        //     LOG_DEBUG ("parser rcv count times %zu", atomic_load(&prsr->count));
        // }

        LOG_DEBUG ("parser recv: %d/%d,\taddr: %p",
            pkt->hdr.caplen, pkt->hdr.len, pkt->data);
        // packet_dump(pkt);

        // packet_match_words(pkt);
        MEMREF text = { pkt->data, pkt->hdr.caplen };
        word_policy_match(&prsr->wordp, text);

        // packet_match_regexs(pkt);
        rc = re_policy_match(&prsr->rep, pkt->data, pkt->hdr.caplen, re_match);
        if (0 != rc) {
            for (int i=0; i<rc; i++) {
                LOG_DEBUG ("re_policy_match %d patterns  --> %d: %s",
                        rc, re_match[i], prsr->rep.rules[re_match[i]].pattern);
            }
            atomic_fetch_add(&prsr->regex_match_count, rc);
        }

        packet_free(pkt);
    }
    LOG_INFO ("END parser loop, parser count %zu", atomic_load(&prsr->count));

    return ((void*)0);
}

inline void parser_time_ev(struct parser* prsr, int seconds) {
    prsr->timer_interval = seconds;
}

static int match_word_cb(int strnum, int textpos, MEMREF const *pattv)
{
    (void)strnum, (void)textpos, (void)pattv;
    atomic_fetch_add(&prsr->word_match_count, 1);
    LOG_DEBUG ("match word: %9d %7d '%.*s'", textpos, strnum,
                (int)pattv[strnum].len, pattv[strnum].ptr);
    return 0;
}

// dump parser to json string, remember to free sds
sds parser_dump(const struct parser* prsr)
{
    sds s = sdsnew("{");
    s = sdscatprintf(s, " \"pkts\": %llu, \"bytes\": %llu, "
        "\"word_match_count\": %llu, \"regex_match_count\": %llu, "
        " \"chan_pkt\": { \"alloc\": %d, \"used\": %d } }",
        atomic_load(&prsr->count), atomic_load(&prsr->bytes),
        atomic_load(&prsr->word_match_count), atomic_load(&prsr->regex_match_count),
        prsr->chan_pkt->alloc, prsr->chan_pkt->used
    );
    //s = sdscat(s, "}");
    return s;
}
