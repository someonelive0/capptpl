#include "apptpl_init.h"

#include "argparse.h"
#include "version.h"

#include "myconfig.h"
#include "capture.h"


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
        SHOW_VERSION_LOCAL();
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
