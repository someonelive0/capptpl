

#include <pcap.h>

#include "logger.h"


pcap_t* capture_handle = NULL;

int capture_open_device(const char *device) {

  bpf_u_int32 net_mask = 0;
  bpf_u_int32 net_ip = 0;
  char errbuf[PCAP_ERRBUF_SIZE];

  // Check Libpcap version number
  LOG_INFO ("libpcap version: %s", pcap_lib_version());

  pcap_t* handle = pcap_create(device, errbuf);
  if (handle == NULL) {
    LOG_ERROR ("pcap_create device=%s failed:%s", device, errbuf);
    return -1;
  }

  // 设置抓包长度
  int ret = pcap_set_snaplen(handle, 65535);
  if (ret < 0) {
    LOG_WARN ("pcap_set_snaplen %d failed:%s", 65535, pcap_geterr(handle));
  }

  // 设置混杂模式
  ret = pcap_set_promisc(handle, 1);
  if (ret < 0) {
    LOG_WARN ("pcap_set_promisc %d failed:%s", 1, pcap_geterr(handle));
  }

  // 设置读取超时，单位 毫秒
  ret = pcap_set_timeout(handle, 100);
  if (ret < 0) {
    LOG_WARN ("pcap_set_timeout %d failed:%s", 100, pcap_geterr(handle));
  }

  // 设置非立即模式. 1: 立即模式会快速返回，但是会丢包。0: 非立即模式，会缓存一下再读取，丢包少
#ifdef WIN32
#else
  ret = pcap_set_immediate_mode(handle, 0);
  if (ret < 0) {
    LOG_WARN ("pcap_set_immediate_mode %d failed:%s", 0, pcap_geterr(handle));
  }
#endif

    if(pcap_set_buffer_size(handle, 2048) != 0) {
        LOG_WARN ("pcap_set_buffer_size failed, size %d KB, %s",
            2048, pcap_geterr(handle));
    }

  // 激活句柄
  ret = pcap_activate(handle);
  if (ret < 0) {
    LOG_ERROR ("pcap_activate failed:%s", pcap_geterr(handle));
    pcap_close(handle);
    return -1;
  }
  LOG_INFO("pcap snapshot: %d", pcap_snapshot(handle));

  // WIN32 必须在设备activate后设置pcap_setbuff，否则失败
#ifdef _WIN32
  ret = pcap_setmintocopy(handle, 16000);
  if (ret < 0) {
    LOG_WARN ("pcap_setmintocopy %d failed:%s", 16000, pcap_geterr(handle));
  }
#endif

  // step2 获取MAC地址
  if (pcap_lookupnet(device, &net_ip, &net_mask, errbuf) < 0) {
    //* Well, we can't get the netmask for this interface; it's used
    //* only for filters that check for broadcast IP addresses, so
    //* we just punt and use 0.  It might be nice to warn the user,
    //* but that's a pain in a GUI application, as it'd involve popping
    //* up a message box, and it's not clear how often this would make
    //* a difference (only filters that check for IP broadcast addresses
    //* use the netmask).
    //  cmdarg_err("Warning:  Couldn't obtain netmask info (%s).", lookup_net_err_str);
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

#ifdef HAVE_PCAP_SET_IMMEDIATE_MODE
  pcap_set_immediate_mode(1);
#endif

  LOG_INFO("capturer open device success:%s", device);

    capture_handle = handle;
  return 0;
}

int capture_close() {
    if (capture_handle != NULL) {
        pcap_breakloop(capture_handle);
        pcap_close(capture_handle);
        capture_handle = NULL;
    }
    return 0;
}

void capture_cb(u_char *arg, const struct pcap_pkthdr *pkthdr,
                             const u_char *pktdata) {
//   PcapCapturer *cap = (PcapCapturer *)param;
//   cap->ProcessPcapPacket(packet_header, packet_content);
    printf("capture_cb: %d\n", pkthdr->caplen);
}

void* capture(void *arg) {

    LOG_INFO ("capture begin loop");
    int rc = pcap_dispatch(capture_handle, -1, capture_cb, (u_char *)arg);
    if (rc == -1) {
        LOG_ERROR ("pcap_dispatch pcap_geterr:%s", pcap_geterr(capture_handle));
    }
    LOG_INFO ("END capture loop");

    return ((void*)0);
}

