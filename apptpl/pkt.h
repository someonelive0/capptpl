#ifndef PKT_H
#define PKT_H

#include <pcap.h>


struct packet {
    struct pcap_pkthdr *hdr;
    u_char             *data;
};

struct packet* packet_new(const struct pcap_pkthdr *pkthdr, const u_char *pktdata);
int packet_free(struct packet* pkt);
void packet_dump(const struct packet* pkt);

#endif // PKT_H
