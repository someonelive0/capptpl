/*
 * Wrapper libpcap or npcap on windows
 */
#include <stdlib.h>
#include <errno.h>
#include <pcap.h>

// struct ether_header in /usr/include/sys/ethernet.h
// struct in_addr in /usr/include/netinet/in.h
// struct ip in /usr/include/netinet/ip.h
// struct udphdr in /usr/include/netinet/udp.h
// struct tcphdr in /usr/include/netinet/tcp.h

#include "logger.h"


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

int capture_close()
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
