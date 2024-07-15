#ifndef INIT_H
#define INIT_H

#include <stdint.h>

#define DEFAULT_CONFIG_FILE "apptpl2.ini"


struct config {
    char version[8];
    uint16_t  http_port;
    char redis_host[46]; // max 45 bytes, such as 0000:0000:0000:0000:0000:ffff:192.168.100.228
    uint16_t  redis_port;
    char redis_passwd[64];
};

int ini_callback(void* arg, const char* section, const char* name, const char* value);
int parse_args(int argc, const char** argv, int* debug, const char** config_filename);


#endif // INIT_H
