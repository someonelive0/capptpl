#ifndef CAPTURE_H
#define CAPTURE_H

#include <pcap.h>


struct capture {
    const char*    device;
    const char*    filter;
    int      snaplen;
    int      buffer_size;

    int      shutdown;
    uint64_t count;
    pcap_t*  handle;
    struct bpf_program* bpf;
};

int capture_open(struct capture* captr, const char *device, 
            int snaplen, int buffer_size, const char* filter);
int capture_close(struct capture* captr);
// arg is struct capture*
void* capture_loop(void *arg);
// when capture packet to callback function;
void pkt_cb(u_char *arg, const struct pcap_pkthdr *pkthdr, const u_char *pktdata);

// =============================================
// not with struct
// =============================================
extern int capture_shutdown;

int capture_open_device(const char *device, int snaplen, int buffer_size, const char* filter);
int capture_close_device();
void* capture(void *arg);
int list_devices();

#endif // CAPTURE_H
