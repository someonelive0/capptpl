#ifndef CAPTURE_H
#define CAPTURE_H

#include <pcap.h>

#include "cchan_pthread.h"


struct capture {
    cchan_t       *chan_pkt;
    const char*    device;
    const char*    filter;
    int      snaplen;
    int      buffer_size;

    int      shutdown;
    uint64_t pkts;     // total packet number
    uint64_t bytes;    // total byte number
    pcap_t*  handle;
    struct bpf_program* bpf;
};

int capture_open(struct capture* captr, const char *device, 
            int snaplen, int buffer_size, const char* filter);
int capture_close(struct capture* captr);
int capture_stats(struct capture* captr);

// arg is struct capture*
void* capture_loop(void *arg);
// when capture packet to callback function;
void pkt_cb(u_char *arg, const struct pcap_pkthdr *pkthdr, const u_char *pktdata);

// =============================================
// not with struct
// =============================================
#if 0
extern int capture_shutdown;

int capture_open_device(const char *device, int snaplen, int buffer_size, const char* filter);
int capture_close_device();
void* capture(void *arg);
#endif

int list_devices();

#endif // CAPTURE_H
