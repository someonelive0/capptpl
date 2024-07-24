#include <stdlib.h>

#include "logger.h"

#include "pkt.h"
#include "parser.h"


// arg is struct parser*
void* parser_loop(void *arg)
{
    struct packet *pkt;

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

        printf("parser recv: %d/%d,\taddr: %p, %p\n",
            pkt->hdr->caplen, pkt->hdr->len, pkt->hdr, pkt->data);

        // LOG_TRACE ("parser rcv pkt %d, %d\n", pkt->hdr, pkt->hdr);
        prsr->count ++;
        if ((prsr->count % 10000) == 0) {
            LOG_DEBUG ("worker rcv count times %zu", prsr->count);
        }

        packet_free(pkt);
    }
    LOG_INFO ("END parser loop, parser count %zu", prsr->count);

    return ((void*)0);
}