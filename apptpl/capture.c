/*
 * Wrapper libpcap or npcap on windows
 */
#include "capture.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

// struct ether_header in /usr/include/sys/ethernet.h
// struct in_addr in /usr/include/netinet/in.h
// struct ip in /usr/include/netinet/ip.h
// struct udphdr in /usr/include/netinet/udp.h
// struct tcphdr in /usr/include/netinet/tcp.h

#include "logger.h"

#include "pkt.h"


int capture_open(struct capture* captr, const char *device, 
            int snaplen, int buffer_size, const char* filter)
{
    pcap_t* handle = NULL;
    struct bpf_program* bpf = NULL;
    char errbuf[PCAP_ERRBUF_SIZE];
    int datalink = 0;

    if (captr->handle) {
        LOG_ERROR ("capture had opened");
        return -1;
    }

    // Check Libpcap version number
    LOG_DEBUG ("libpcap version: %s", pcap_lib_version());

    if (NULL == (handle = pcap_create(device, errbuf))) {
        LOG_ERROR ("pcap_create device=%s failed:%s", device, errbuf);
        return -1;
    }

    // 设置抓包长度
    if (0 != pcap_set_snaplen(handle, snaplen)) {
        LOG_WARN ("pcap_set_snaplen %d failed:%s", snaplen, pcap_geterr(handle));
    }

    // 设置混杂模式
    if (0 != pcap_set_promisc(handle, 1)) {
        LOG_WARN ("pcap_set_promisc %d failed:%s", 1, pcap_geterr(handle));
    }

    // 设置读取超时，单位 毫秒, 默认 100ms
    if (0 != pcap_set_timeout(handle, 100)) {
        LOG_WARN ("pcap_set_timeout %d failed:%s", 100, pcap_geterr(handle));
    }

    // 设置非立即模式. 1: 立即模式会快速返回，但是会丢包。0: 非立即模式，会缓存一下再读取，丢包少
    if (0 != pcap_set_immediate_mode(handle, 0)) {
        LOG_WARN ("pcap_set_immediate_mode %d failed:%s", 0, pcap_geterr(handle));
    }

    // 设置缓存大小，单位 KBytes, pcap_set_buffer_size() in bytes.
    if (0 != pcap_set_buffer_size(handle, buffer_size * 1024)) {
        LOG_WARN ("pcap_set_buffer_size failed, size %d KB, %s",
                  buffer_size, pcap_geterr(handle));
    }
    LOG_INFO("pcap buffer_size: %d KBytes", buffer_size);

    // 激活句柄
    if (0 != pcap_activate(handle)) {
        LOG_ERROR ("pcap_activate '%s' failed: %s", device, pcap_geterr(handle));
        if (NULL != strstr(pcap_geterr(handle), "Operation not permitted"))
            LOG_WARN("fix not permitted to run:\n\tsudo setcap 'CAP_NET_RAW,CAP_NET_ADMIN,CAP_DAC_OVERRIDE+ep' apptpl");
        goto err;
    }
    LOG_INFO ("pcap snapshot: %d", pcap_snapshot(handle));


    if (strlen(filter) > 0) {
        if (NULL == (bpf = malloc(sizeof(struct bpf_program)))) {
            LOG_ERROR ("capturer malloc filter of struct bpf_program");
            goto err;
        }
        if (0 != pcap_compile(handle, bpf, filter, 1, 0xFFFFFF00)) {
            LOG_ERROR ("capturer pcap_compile filter:%s, failed:%s", filter, pcap_geterr(handle));
            goto err;
        }
        /*设置FILTER*/
        if (0 != pcap_setfilter(handle, bpf)) {
            LOG_ERROR ("capturer pcap_setfilter failed:%s", pcap_geterr(handle));
            goto err;
        }
        LOG_INFO("pcap filter: %s", filter);
    }

    // WIN32 必须在设备activate后设置pcap_setbuff，否则失败
#ifdef _WIN32
    if (0 != pcap_setmintocopy(handle, 16000)) {
        LOG_WARN ("pcap_setmintocopy %d failed:%s", 16000, pcap_geterr(handle));
    }
#endif

    // step2 查看网卡类型
    /*
    * datalink:
    *  DLT_EN10MB: Ethernet (10Mb, 100Mb, 1000Mb, and up)
    *  DLT_NULL: BSD loopback encapsulation; the link layer header is a 4-byte
    * field, in host byte order, containing a PF_ value from socket.h for the
    * network-layer protocol of the packet. DLT_LINUX_SLL: Linux "cooked"
    * capture encapsulation;
    */
    datalink = pcap_datalink(handle);
    if (datalink != DLT_EN10MB && datalink != DLT_NULL) {
        LOG_ERROR ("capturer device=%s data link is %d, only support ethernet type",
                   device, datalink);
        goto err;
    }

    LOG_INFO ("capturer open device success: %s", device);
    goto done;

err:
    if (handle) pcap_close(handle);
    if (bpf) {
        pcap_freecode(bpf);
        free(bpf);
    }
    return -1;

done:
    captr->handle = handle;
    captr->bpf = bpf;
    captr->device = device;
    captr->filter = filter;
    captr->snaplen = snaplen;
    captr->buffer_size = buffer_size;
    return 0;
}

int capture_close(struct capture* captr)
{
    if (captr->handle != NULL) {
        pcap_breakloop(captr->handle);
        pcap_close(captr->handle);
        captr->handle = NULL;
    }
    if (captr->bpf != NULL) {
        pcap_freecode(captr->bpf);
        free(captr->bpf);
        captr->bpf = NULL;
    }
    return 0;
}

/*
 * WIN32 to use pcap_stats_ex() to get more pcap stat
 * struct pcap_stat on linux has 12 bytes, and win32 has 24 bytes.
 */
int capture_stats(struct capture* captr)
{
    if (captr->handle == NULL) {
        LOG_ERROR ("capturer pcap_stats failed, handle is not opened");
        return -1;
    }

#ifdef _WIN32
    struct pcap_stat* ps = NULL;
    int len = 0;
    if (NULL == (ps = pcap_stats_ex(captr->handle, &len))) {
        LOG_ERROR ("capturer pcap_stats_ex failed:%s", pcap_geterr(captr->handle));
        return -1;
    }
    memcpy(&captr->ps, ps, sizeof(struct pcap_stat));
    // printf("pcap_stats_ex len %d, %zu\n", len, sizeof(struct pcap_stat)); // win32 is 24 bytes.
#else
    if (0 != pcap_stats(captr->handle, &captr->ps)) {
        LOG_ERROR ("capturer pcap_stats failed:%s", pcap_geterr(captr->handle));
        return -1;
    }
#endif

    return 0;
}

// arg is struct capture*
void* capture_loop(void *arg)
{
    struct capture* captr = arg;
    int rc;
    captr->shutdown = 0;
    captr->pkts = 0;
    captr->bytes = 0;

    LOG_INFO ("capture begin loop, size of struct pcap_pkthdr %zu, struct packet %zu, struct pcap_stat %zu",
                sizeof(struct pcap_pkthdr), sizeof(struct packet), sizeof(struct pcap_stat));
    while (!captr->shutdown) {
        rc = pcap_dispatch(captr->handle, -1, pkt_cb, (u_char *)captr);
        if (rc == -1) {
            LOG_ERROR ("pcap_dispatch pcap_geterr:%s", pcap_geterr(captr->handle));
            break;
        }
    }

    if (0 == capture_stats(captr)) {
        LOG_INFO ("pcap stat, ps_recv=%d, ps_drop=%d, ps_ifdrop=%d",
                captr->ps.ps_recv, captr->ps.ps_drop, captr->ps.ps_ifdrop);
#ifdef _WIN32
        LOG_INFO ("pcap stat_ex, ps_capt=%d, ps_sent=%d, ps_netdrop=%d",
                captr->ps.ps_capt, captr->ps.ps_sent, captr->ps.ps_netdrop);
#endif
    }
    LOG_INFO ("END capture loop, capture pkts %zu, bytes %zu", captr->pkts, captr->bytes);

    return ((void*)0);
}

int capture_loop_stop(struct capture* captr)
{
    if (captr->handle != NULL) {
        captr->shutdown = 1;
        pcap_breakloop(captr->handle);
    }
    return 0;
}

void pkt_cb(u_char *arg, const struct pcap_pkthdr *pkthdr, const u_char *pktdata)
{
    struct capture* captr = (struct capture*)arg;
    captr->pkts ++;
    captr->bytes += pkthdr->caplen;
    // printf("pkt_callback: %d/%d,\taddr: %p, %p\n",
    //        pkthdr->caplen, pkthdr->len, pkthdr, pktdata);
    
    struct packet* pkt = packet_new(pkthdr, pktdata);
    if (NULL == pkt) {
        LOG_ERROR ("packet_new failed");
    } else {
        cchan_send(captr->chan_pkt, &pkt);

        // to limit channel of pkt size
        if (captr->chan_pkt->used >= 1024) {
            // LOG_TRACE ("chan_pkt used size: %d, %d\n", chan_pkt->used, chan_pkt->size);
            usleep(100);
        }
    }
}

// dump capture to json string, remember to free sds
sds capture_dump(struct capture* captr)
{
    char device[160] = {0};
    for (size_t i=0, j=0; i<strlen(captr->device) && i<sizeof(device)-1; i++, j++) {
        if (captr->device[i] == '\\' || captr->device[i] == '"')
            device[j++] = '\\';
        device[j] = captr->device[i];
    }

    capture_stats(captr);

    sds s = sdsnew("{");
    s = sdscatprintf(s, " \"pkts\": %" PRIu64 ", \"bytes\": %" PRIu64 ", "
        "\"pcap\": { \"ps_recv\": %u, \"ps_drop\": %u, \"ps_ifdrop\": %u"
#ifdef _WIN32
        ", \"ps_capt\": %d, \"ps_sent\": %d, \"ps_netdrop\": %d"
#endif
        " }, \"device\": \"%s\", \"snaplen\": %d, \"buffer_size\": %d, \"filter\": \"%s\" "
        "}",
        captr->pkts, captr->bytes,
        captr->ps.ps_recv, captr->ps.ps_drop, captr->ps.ps_ifdrop,
#ifdef _WIN32
        captr->ps.ps_capt, captr->ps.ps_sent, captr->ps.ps_netdrop,
#endif
        device, captr->snaplen, captr->buffer_size, captr->filter
    );
    //s = sdscat(s, "}");
    return s;
}


#if 0
// =============================================
// not with struct
// =============================================
int capture_shutdown;
static uint64_t capture_count;
static pcap_t* capture_handle = NULL;
static struct bpf_program* capture_bpf = NULL;

int capture_open_device(const char *device, int snaplen, int buffer_size, const char* filter)
{

    bpf_u_int32 net_mask = 0;
    bpf_u_int32 net_ip = 0;
    char errbuf[PCAP_ERRBUF_SIZE];

    // Check Libpcap version number
    LOG_DEBUG ("libpcap version: %s", pcap_lib_version());

    pcap_t* handle = pcap_create(device, errbuf);
    if (handle == NULL) {
        LOG_ERROR ("pcap_create device=%s failed:%s", device, errbuf);
        return -1;
    }

    // 设置抓包长度
    int rc = pcap_set_snaplen(handle, snaplen);
    if (rc < 0) {
        LOG_WARN ("pcap_set_snaplen %d failed:%s", snaplen, pcap_geterr(handle));
    }

    // 设置混杂模式
    rc = pcap_set_promisc(handle, 1);
    if (rc < 0) {
        LOG_WARN ("pcap_set_promisc %d failed:%s", 1, pcap_geterr(handle));
    }

    // 设置读取超时，单位 毫秒, 默认 100ms
    rc = pcap_set_timeout(handle, 100);
    if (rc < 0) {
        LOG_WARN ("pcap_set_timeout %d failed:%s", 100, pcap_geterr(handle));
    }

    // 设置非立即模式. 1: 立即模式会快速返回，但是会丢包。0: 非立即模式，会缓存一下再读取，丢包少
    rc = pcap_set_immediate_mode(handle, 0);
    if (rc < 0) {
        LOG_WARN ("pcap_set_immediate_mode %d failed:%s", 0, pcap_geterr(handle));
    }

    // 设置缓存大小，单位 KBytes
    if (pcap_set_buffer_size(handle, buffer_size) != 0) {
        LOG_WARN ("pcap_set_buffer_size failed, size %d KB, %s",
                  buffer_size, pcap_geterr(handle));
    }
    LOG_INFO("pcap buffer_size: %d KBytes", buffer_size);

    // 激活句柄
    rc = pcap_activate(handle);
    if (rc < 0) {
        LOG_ERROR ("pcap_activate failed:%s", pcap_geterr(handle));
        pcap_close(handle);
        return -1;
    }
    LOG_INFO ("pcap snapshot: %d", pcap_snapshot(handle));


    if (strlen(filter) > 0) {
        if (NULL == (capture_bpf = malloc(sizeof(struct bpf_program)))) {
            LOG_ERROR ("capturer malloc filter of struct bpf_program");
            return -1;
        }
        if (pcap_compile(handle, capture_bpf, filter, 1, 0xFFFFFF00) == -1) {
            LOG_ERROR ("capturer pcap_compile filter:%s, failed:%s", filter, pcap_geterr(handle));
            return -1;
        }
        /*设置FILTER*/
        if (pcap_setfilter(handle, capture_bpf) == -1) {
            LOG_ERROR ("capturer pcap_setfilter failed:%s", pcap_geterr(handle));
            return -1;
        }
        LOG_INFO("pcap filter: %s", filter);
    }

    // WIN32 必须在设备activate后设置pcap_setbuff，否则失败
#ifdef _WIN32
    rc = pcap_setmintocopy(handle, 16000);
    if (rc < 0) {
        LOG_WARN ("pcap_setmintocopy %d failed:%s", 16000, pcap_geterr(handle));
    }
#endif

    // step2 获取MAC地址
    if (pcap_lookupnet(device, &net_ip, &net_mask, errbuf) < 0) {
        net_mask = 0;
    }

    // step3 查看网卡类型
    /*
    * datalink:
    *  DLT_EN10MB: Ethernet (10Mb, 100Mb, 1000Mb, and up)
    *  DLT_NULL: BSD loopback encapsulation; the link layer header is a 4-byte
    * field, in host byte order, containing a PF_ value from socket.h for the
    * network-layer protocol of the packet. DLT_LINUX_SLL: Linux "cooked"
    * capture encapsulation;
    */
    int datalink = pcap_datalink(handle);
    if (datalink != DLT_EN10MB && datalink != DLT_NULL) {
        LOG_ERROR ("capturer device=%s data link is %d, only support ethernet type",
                   device, datalink);
        pcap_close(handle);
        return -1;
    }

    LOG_INFO ("capturer open device success: %s", device);

    capture_handle = handle;
    return 0;
}

int capture_close_device()
{
    if (capture_handle != NULL) {
        pcap_breakloop(capture_handle);
        pcap_close(capture_handle);
        capture_handle = NULL;
    }
    if (capture_bpf != NULL) {
        pcap_freecode(capture_bpf);
        free(capture_bpf);
        capture_bpf = NULL;
    }
    return 0;
}

#define UNUSED(x) (void)(x)
static void capture_cb(u_char *arg, const struct pcap_pkthdr *pkthdr, const u_char *pktdata)
{
    UNUSED(arg);
    capture_count ++;
    // PcapCapturer *cap = (PcapCapturer *)arg;
    // cap->ProcessPcapPacket(packet_header, packet_content);
    printf("capture_cb: %d/%d,\tdata addr: %p\n",
           pkthdr->caplen, pkthdr->len, &pktdata);
}

/*
 * loop to cature packets, can be called by pthread
 */
void* capture(void *arg)
{
    int rc;

    LOG_INFO ("capture begin loop");
    capture_shutdown = 0;
    capture_count = 0;
    while (!capture_shutdown) {
        rc = pcap_dispatch(capture_handle, -1, capture_cb, (u_char *)arg);
        if (rc == -1) {
            LOG_ERROR ("pcap_dispatch pcap_geterr:%s", pcap_geterr(capture_handle));
            break;
        }
    }
    LOG_INFO ("END capture loop, capture count %lld", capture_count);

    return ((void*)0);
}
#endif

int list_devices()
{
    char errbuf[PCAP_ERRBUF_SIZE]; /* Size defined in pcap.h */
    pcap_if_t *alldevs, *d;

    char ip[46] = {0}; // include ipv6
    char subnet_mask[46] = {0};
    bpf_u_int32 subnet_raw; /* Subnet address as integer */
    bpf_u_int32 subnet_mask_raw; /* Subnet mask as integer */
    struct in_addr inaddr; /* Used for both ip & subnet */

    if (pcap_findalldevs(&alldevs, errbuf) == -1) {
        printf ("pcap_findalldevs failed. error=%s\n", errbuf);
        return -1;
    }

    /* Scan the list printing every entry */
    for (d = alldevs; d; d = d->next) {
        if (d->description)
            printf("name: %s\n", d->description);
        else
            printf("name: None\n");
        printf("\tdevice: %s\n", d->name);

        for (pcap_addr_t *a = d->addresses; a; a = a->next) {
            if (a->addr && a->addr->sa_family == AF_INET) {
                memset(ip, 0, sizeof(ip));
                inet_ntop(AF_INET, &((struct sockaddr_in *)(a->addr))->sin_addr.s_addr, ip, sizeof(ip));
                printf("\tipv4: %s\n", ip);
            } else if (a->addr && a->addr->sa_family == AF_INET6) {
                memset(ip, 0, sizeof(ip));
                inet_ntop(AF_INET6, &((struct sockaddr_in6 *)(a->addr))->sin6_addr, ip, sizeof(ip));
                printf("\tipv6: %s\n", ip);
            } else {
                // printf("other sa_family:%d\n", a->addr->sa_family);
                continue;
            }
        }

        /* Get device subnet info */
        if (-1 == pcap_lookupnet(d->name, &subnet_raw, &subnet_mask_raw, errbuf)) {
            //printf ("\tpcap_lookupnet of device %s failed: %s\n", d->name, errbuf);
            continue;
        }

        /* Get ip in human readable form */
        inaddr.s_addr = subnet_raw;
        char* p = inet_ntoa(inaddr);
        if (p == NULL) {
            printf("inet_ntoa ip failed, %d %s\n", errno, strerror(errno));
            return -1;
        }
        strncpy(ip, p, sizeof(ip)-1);

        /* Get subnet mask in human readable form */
        inaddr.s_addr = subnet_mask_raw;
        if ((p = inet_ntoa(inaddr)) == NULL) {
            printf("inet_ntoa netmask failed, %d %s\n", errno, strerror(errno));
            return -1;
        }
        strncpy(subnet_mask, p, sizeof(subnet_mask)-1);

        printf("\tsubnet: %s\tnetmask: %s\n", ip, subnet_mask);
    }

    pcap_freealldevs(alldevs);

    return 0;
}
