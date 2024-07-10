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
};

int ini_callback(void* arg, const char* section, const char* name, const char* value)
{
    struct config* pconfig = (struct config*)arg;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("global", "version")) {
        LOG_DEBUG ("ini global %s, name %s, value %s", section, name, value);
        strncpy(pconfig->version, value, sizeof(pconfig->version)-1);
    } else if (MATCH("http", "port")) {
        LOG_DEBUG ("ini section %s, name %s, value %s", section, name, value);
        pconfig->http_port = atoi(value);
    } else if (MATCH("zmq", "port")) {
        LOG_DEBUG ("ini section %s, name %s, value %s", section, name, value);
        pconfig->zmq_port = atoi(value);
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

int parse_args(int argc, const char** argv, int* debug, char** config_filename) {
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
