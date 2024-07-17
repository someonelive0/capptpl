#include <stdlib.h>

#include "logger.h"
#include "argparse.h"

#include "version.h"
#include "apptpl2_init.h"


int ini_callback(void* arg, const char* section, const char* name, const char* value) {
    struct config* pconfig = (struct config*)arg;

    // LOG_TRACE ("ini global %s, name %s, value %s", section, name, value);

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("global", "version")) {
        strncpy(pconfig->version, value, sizeof(pconfig->version)-1);
    } else if (MATCH("http", "port")) {
        pconfig->http_port = atoi(value);
    } else if (MATCH("redis", "host")) {
        strncpy(pconfig->redis_host, value, sizeof(pconfig->redis_host)-1);
    } else if (MATCH("redis", "port")) {
        pconfig->redis_port = atoi(value);
    } else if (MATCH("redis", "passwd")) {
        strncpy(pconfig->redis_passwd, value, sizeof(pconfig->redis_passwd)-1);
    } else {
        LOG_ERROR ("unknown ini section %s, name %s, value %s", section, name, value);
        return 0;  /* unknown section/name, error */
    }
    return 1;
}


static const char *const usages[] = {
    "appctl2 [options] [[--] args]",
    "appctl2 [options]",
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
        SHOW_VERSION();
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
