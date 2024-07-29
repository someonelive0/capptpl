#include <stdlib.h>
#include <string.h>

#include "hex.h"
#include "pkt.h"


struct packet* packet_new(const struct pcap_pkthdr *pkthdr, const u_char *pktdata)
{
    struct packet* pkt = malloc(sizeof(struct packet));
    if (NULL == pkt) return NULL;

    // if (NULL == (pkt->hdr = malloc(sizeof(struct pcap_pkthdr)))) goto err;
    memcpy(&pkt->hdr, pkthdr, sizeof(struct pcap_pkthdr));

    // if (NULL == (pkt->data = malloc(pkthdr->caplen))) goto err;
    // memcpy(pkt->data, pktdata, pkthdr->caplen);
    if (NULL == (pkt->data = sdsnewlen(pktdata, pkthdr->caplen))) goto err;

    return pkt;

err:
    packet_free(pkt);
    return NULL;
}

int packet_free(struct packet* pkt)
{
    if (pkt) {
        // if (pkt->hdr) {
        //     free(pkt->hdr);
        //     pkt->hdr = NULL;
        // }
        if (pkt->data) {
            // free(pkt->data);
            sdsfree(pkt->data);
            pkt->data = NULL;
        }
        free(pkt);
    }

    return 0;
}

void packet_dump(const struct packet* pkt)
{
    printf("caplen=%d, len=%d\n", pkt->hdr.caplen, pkt->hdr.len);
    dump_hex(pkt->data, pkt->hdr.caplen);
}
