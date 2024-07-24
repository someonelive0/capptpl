#ifndef PKT_H
#define PKT_H

#include <pcap.h>

#include "sds.h"


struct packet {
    struct pcap_pkthdr *hdr;
    // u_char             *data;
    sds                data; // typedef char *sds; from sds.h
};

struct packet* packet_new(const struct pcap_pkthdr *pkthdr, const u_char *pktdata);
int packet_free(struct packet* pkt);
void packet_dump(const struct packet* pkt);

#endif // PKT_H
