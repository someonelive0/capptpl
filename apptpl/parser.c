#include <stdlib.h>
// #include <stdatomic.h>

#include "logger.h"

#include "pkt.h"
#include "parser.h"


// arg is struct parser*
void* parser_loop(void *arg)
{
    struct packet *pkt = NULL;

    struct parser* prsr = arg;
    prsr->shutdown = 0;
    prsr->count = 0;
    prsr->bytes = 0;

    LOG_INFO ("parser loop");
    while (!prsr->shutdown) {
        // process timer_seconds first
        if (prsr->timer_interval) {
            LOG_DEBUG ("parser timer interval seconds %d, rcv count %zu, bytes %zu",
                        prsr->timer_interval, prsr->count, prsr->bytes);
            prsr->timer_interval = 0;
        }

        if (0 == cchan_waittime(prsr->chan_pkt, &pkt, PARSER_WAIT_MS)) { // 0 means timeout
            // printf("cchan_waittime\n");
            //if (chan_pkt->used > 10000) {
            // printf("chan_pkt used, alloc: %d, %d\n", chan_pkt->used, chan_pkt->alloc);
            //}
            continue;
        }

        prsr->count ++;
        prsr->bytes += pkt->hdr->caplen;
        if ((prsr->count % 10000) == 0) {
            LOG_DEBUG ("parser rcv count times %zu", prsr->count);
        }

        LOG_DEBUG ("parser recv: %d/%d,\taddr: %p, %p",
            pkt->hdr->caplen, pkt->hdr->len, pkt->hdr, pkt->data);
        // packet_dump(pkt);

        packet_free(pkt);
    }
    LOG_INFO ("END parser loop, parser count %zu", prsr->count);

    return ((void*)0);
}
