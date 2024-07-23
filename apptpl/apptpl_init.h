#ifndef INIT_H
#define INIT_H

#include <stdint.h>

#include "utstring.h"

#define DEFAULT_CONFIG_FILE "apptpl.ini"


struct config {
    char version[8];
    uint16_t http_port;
    uint16_t zmq_port;
    char pcap_device[128];
    int  pcap_snaplen;
    int  pcap_buffer_size;
    char pcap_filter[256];
};

UT_string* config2json(const struct config* p);

int ini_callback(void* arg, const char* section, const char* name, const char* value);
int parse_args(int argc, const char** argv, int* debug, const char** config_filename);

#endif // INIT_H
