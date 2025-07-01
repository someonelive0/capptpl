#include "apptpl2_init.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "logger.h"
#include "argparse.h"
#include "version.h"


UT_string* config2json(const struct config* p)
{
    UT_string *s;
    utstring_new(s);
    utstring_printf(s, "{\"version\": \"%s\", \"http_port\": %d, "
"\"enable_ssl\": %d, \"crt_file\": \"%s\", \"key_file\": \"%s\", "
"\"redis_host\": \"%s\",  \"redis_port\": %d}",
        p->version, p->http_port, 
        p->enable_ssl, p->crt_file, p->key_file,
        p->redis_host, p->redis_port);
    // printf("%s\n", utstring_body(s));

    // should remember to free s by utstring_free(s);
    return s;
}

int ini_callback(void* arg, const char* section, const char* name, const char* value)
{
    struct config* pconfig = (struct config*)arg;

    // LOG_TRACE ("ini global %s, name %s, value %s", section, name, value);

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("global", "version")) {
        strncpy(pconfig->version, value, sizeof(pconfig->version)-1);
    } else if (MATCH("http", "port")) {
        pconfig->http_port = atoi(value);
    } else if (MATCH("http", "enable_ssl")) {
        if (0 == strcasecmp(value, "true") || 0 == strcmp(value, "1")) {
            pconfig->enable_ssl = 1;
        } else {
            pconfig->enable_ssl = 0;
        }
    } else if (MATCH("http", "crt_file")) {
        strncpy(pconfig->crt_file, value, sizeof(pconfig->crt_file)-1);
    } else if (MATCH("http", "key_file")) {
        strncpy(pconfig->key_file, value, sizeof(pconfig->crt_file)-1);
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

int parse_args(int argc, const char** argv, int* debug, const char** config_filename)
{
    int version = 0;
    int list = 0;
    // int debug = 0;
    // const char *config_filename = NULL;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Basic options"),
        OPT_BOOLEAN('v', "version", &version, "show version", NULL, 0, 0),
        OPT_BOOLEAN('l', "list", &list, "list network devices", NULL, 0, 0),
        // OPT_BOOLEAN('D', "debug", debug, "set log level to debug", NULL, 0, 0),
        OPT_INTEGER('D', "debug", debug, 
            "set log level, TRACE=0, DEBUG=1, INFO=2, WARN=3, ERROR=4, FATAL=5, default is INFO",
            NULL, 0, 0),
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

    if (argc != 0) {
        printf("there are more unknown argc %d: ", argc);
        for (int i = 0; i < argc; i++) {
            printf("argv[%d]: %s, ", i, *(argv + i));
        }
        printf("\n");
        return -1;
    }

    return 0;
}

void restart(char** argv)
{
    // execve("/proc/self/exe", argv, NULL);
    int rc = execv("/proc/self/exe", argv);
    if (rc == -1) {
        printf("restart with execv failed, %d: %s\n", errno, strerror(errno));
    }
}
