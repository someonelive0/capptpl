#include "apptpl_init.h"

#include <stdlib.h>

#include "logger.h"
#include "argparse.h"
#include "version.h"
#include "load_config.h"

#include "capture.h"


UT_string* config2json(const struct config* p)
{
    UT_string *s;
    char device[160] = {0};
    for (size_t i=0, j=0; i<strlen(p->pcap_device) && i<sizeof(device)-1; i++, j++) {
        if (p->pcap_device[i] == '\\' || p->pcap_device[i] == '"')
            device[j++] = '\\';
        device[j] = p->pcap_device[i];
    }

    utstring_new(s);
    utstring_printf(s, "{\"version\": \"%s\", \"http_port\": %d, "
"\"enable_ssl\": %d, \"crt_file\": \"%s\", \"key_file\": \"%s\", "
"\"zmq_port\": %d, \"pcap_device\": \"%s\", \"pcap_snaplen\": %d, "
"\"pcap_buffer_size\": %d, \"pcap_filter\": \"%s\", "
"\"word_file\": \"%s\", \"regex_file\": \"%s\""
"}",
        p->version, p->http_port, p->enable_ssl, p->crt_file, p->key_file,
        p->zmq_port, device, p->pcap_snaplen, p->pcap_buffer_size, p->pcap_filter,
        p->word_file, p->regex_file);
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
    } else if (MATCH("word_policy", "word_file")) {
        strncpy(pconfig->word_file, value, sizeof(pconfig->word_file)-1);
    } else if (MATCH("regex_policy", "regex_file")) {
        strncpy(pconfig->regex_file, value, sizeof(pconfig->regex_file)-1);
    } else {
        LOG_ERROR ("unknown ini section %s, name %s, value %s", section, name, value);
        return 0;  /* unknown section/name, error */
    }
    return 1;
}


static const char *const usages[] = {
    "appctl [options] [[--] args]",
    "appctl [options]",
    NULL,
};

int parse_args(int argc, const char** argv, int* debug, const char** config_filename)
{
    int version = 0;
    int list = 0;
    int test = 0;
    // const char *config_filename = NULL;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Basic options"),
        OPT_BOOLEAN('v', "version", &version, "show version", NULL, 0, 0),
        OPT_BOOLEAN('l', "list", &list, "list network devices", NULL, 0, 0),
        OPT_BOOLEAN('T', "test", &test, "test configuration file", NULL, 0, 0),
        OPT_INTEGER('D', "debug", debug, 
            "set log level, TRACE=0, DEBUG=1, INFO=2, WARN=3, ERROR=4, FATAL=5, default is INFO",
            NULL, 0, 0),
        OPT_STRING('c', "config", config_filename, "set ini config filename", NULL, 0, 0),
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
    if (list != 0) {
        list_devices();
        return 1;
    }
     if (test != 0) {
        test_config(*config_filename);
        return 1;
    }

    if (argc != 0) {
        printf("there are more unknown argc %d: ", argc);
        int i;
        for (i = 0; i < argc; i++) {
            printf("argv[%d]: %s, ", i, *(argv + i));
        }
        printf("\n");
        return -1;
    }

    return 0;
}

int test_config(const char* config_filename) {
    if (NULL == config_filename) config_filename = DEFAULT_CONFIG_FILE;
    struct config myconfig;
    memset(&myconfig, 0, sizeof(struct config));
    if (load_config_ini(config_filename, ini_callback, &myconfig) < 0) {
        return -1;
    }

    UT_string* s = config2json(&myconfig);
    printf ("%s", utstring_body(s));
    utstring_free(s);

    return 0;
}
