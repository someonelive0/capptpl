#ifndef INIT_H
#define INIT_H

#include "logger.h"
#include "argparse.h"
#include "capture.h"

#define DEFAULT_CONFIG_FILE "apptpl.ini"


struct config {
    char version[8];
    int http_port;
    int zmq_port;
    char pcap_device[128];
    int pcap_snaplen;
    int pcap_buffer_size;
    char pcap_filter[256];
};

int ini_callback(void* arg, const char* section, const char* name, const char* value)
{
    struct config* pconfig = (struct config*)arg;

    LOG_DEBUG ("ini global %s, name %s, value %s", section, name, value);

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("global", "version")) {
        strncpy(pconfig->version, value, sizeof(pconfig->version)-1);
    } else if (MATCH("http", "port")) {
        pconfig->http_port = atoi(value);
    } else if (MATCH("zmq", "port")) {
        pconfig->zmq_port = atoi(value);
    } else if (MATCH("pcap", "device")) {
        strncpy(pconfig->pcap_device, value, sizeof(pconfig->pcap_device)-1);
    } else if (MATCH("pcap", "snaplen")) {
        pconfig->pcap_snaplen = atoi(value);
    } else if (MATCH("pcap", "buffer_size")) {
        pconfig->pcap_buffer_size = atoi(value);
    } else if (MATCH("pcap", "filter")) {
        strncpy(pconfig->pcap_filter, value, sizeof(pconfig->pcap_filter)-1);
    } else {
        LOG_ERROR ("unknown ini section %s, name %s, value %s", section, name, value);
        return 0;  /* unknown section/name, error */
    }
    return 1;
}


const char *const usages[] = {
    "appctl [options] [[--] args]",
    "appctl [options]",
    NULL,
};

int parse_args(int argc, const char** argv, int* debug, const char** config_filename) {
    int version = 0;
    int list = 0;
    // int debug = 0;
    // const char *config_filename = NULL;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Basic options"),
        OPT_BOOLEAN('v', "version", &version, "show version", NULL, 0, 0),
        OPT_BOOLEAN('l', "list", &list, "list network devices", NULL, 0, 0),
        OPT_BOOLEAN('D', "debug", debug, "set log level to debug", NULL, 0, 0),
        OPT_STRING('f', "config", config_filename, "set ini config filename", NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nA brief description of what the program does and how it works.", "\nAdditional description of the program after the description of the arguments.");
    argc = argparse_parse(&argparse, argc, argv);

    if (version != 0) {
        show_version();
        return 1;
    }
    if (list != 0) {
        list_devices();
        return 1;
    }
    // if (debug != 0)
    //     printf("debug: %d\n", *debug);
    //  if (config_filename != NULL)
    //     printf("config_filename: %s\n", *config_filename);

    if (argc != 0) {
        printf("argc: %d\n", argc);
        int i;
        for (i = 0; i < argc; i++) {
            printf("argv[%d]: %s\n", i, *(argv + i));
        }
    }

    return 0;
}

#endif // INIT_H
