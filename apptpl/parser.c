#include <stdlib.h>

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

    LOG_INFO ("parser loop");
    while (!prsr->shutdown) {
        if (0 == cchan_waittime(prsr->chan_pkt, &pkt, PARSER_WAIT_MS)) { // 0 means timeout
            // printf("cchan_waittime\n");
            //if (chan_pkt->used > 10000) {
            // printf("chan_pkt used, alloc: %d, %d\n", chan_pkt->used, chan_pkt->alloc);
            //}
            continue;
        }

        prsr->count ++;
        if ((prsr->count % 10000) == 0) {
            LOG_DEBUG ("worker rcv count times %zu", prsr->count);
        }

        LOG_DEBUG ("parser recv: %d/%d,\taddr: %p, %p",
            pkt->hdr->caplen, pkt->hdr->len, pkt->hdr, pkt->data);
        // packet_dump(pkt);

        packet_free(pkt);
    }
    LOG_INFO ("END parser loop, parser count %zu", prsr->count);

    return ((void*)0);
}
