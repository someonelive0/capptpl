#ifndef MYCONFIG_H
#define MYCONFIG_H

#include <stdint.h>

#include "utstring.h"

#define DEFAULT_CONFIG_FILE "apptpl.ini"


/*
 * struct config
 */
struct config {
    char      version[8];
    uint16_t  http_port;
    uint16_t  enable_ssl;
    char      crt_file[128];
    char      key_file[128];
    uint16_t  zmq_port;
    char      pcap_device[128];
    int       pcap_snaplen;
    int       pcap_buffer_size;
    char      pcap_filter[256];
    char      word_file[256];
    char      regex_file[256];
};

int load_config(struct config* myconfig, const char* config_filename);

int test_config(const char* config_filename);

UT_string* config2json(const struct config* p);

int ini_callback(void* arg, const char* section, const char* name, const char* value);

#endif // MYCONFIG_H
