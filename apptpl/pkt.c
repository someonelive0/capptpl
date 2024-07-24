#include "pkt.h"

struct packet* packet_new(const struct pcap_pkthdr *pkthdr, const u_char *pktdata)
{
    struct packet* pkt = malloc(sizeof(struct packet));
    if (NULL == pkt) return NULL;

    if (NULL == (pkt->hdr = malloc(sizeof(struct pcap_pkthdr)))) goto err;
    if (NULL == (pkt->data = malloc(pkthdr->caplen))) goto err;
    
    memcpy(pkt->hdr, pkthdr, sizeof(struct pcap_pkthdr));
    memcpy(pkt->data, pktdata, pkthdr->caplen);

    return pkt;

err:
    packet_free(pkt);
    return NULL;
}

int packet_free(struct packet* pkt)
{
    if (pkt) {
        if (pkt->hdr) {
            free(pkt->hdr);
            pkt->hdr = NULL;
        }
        if (pkt->data) {
            free(pkt->data);
            pkt->data = NULL;
        }
        free(pkt);
    }

    return 0;
}