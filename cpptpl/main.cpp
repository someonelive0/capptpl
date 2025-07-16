/*
 * main.cpp for cpptpl
 */
extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "cchan_pthread.h"
#include "logger.h"

#include "init_log.h"
#include "load_config.h"
}

#include "cpptpl_init.h"
#include "magt.h"


int main(int argc, char* argv[])
{
    if (0 != ch_exec_cwd(argv[0]))
        exit(1);

    struct app myapp = { time(NULL), NULL, NULL, NULL, NULL, NULL };
    int debug = LogLevel_INFO; // default LogLevel_INFO=2
    const char *config_filename = NULL;
    if (0 != parse_args(argc, (const char**)argv, &debug, &config_filename)) {
        exit(1);
    }
    if (NULL == config_filename) config_filename = DEFAULT_CONFIG_FILE;

    if (0 != init_log("cpptpl.log", debug, 1024*1024, 5)) { // call logger_close() to free
        exit(1);
    }

    struct config myconfig;
    memset(&myconfig, 0, sizeof(struct config));
    if (load_config_ini(config_filename, ini_callback, &myconfig) < 0) {
        logger_close();
        exit(1);
    }
    myapp.myconfig = &myconfig;
    UT_string* s = config2json(&myconfig);
    LOG_INFO ("BEGIN at %s\tconfig_filename=%s\t%s",
              asctime(localtime( &myapp.run_time )), // ctime(&myapp.run_time),
              config_filename, utstring_body(s));
    utstring_free(s);

    // init something
    if (-1 == magt_init(&myapp)) {
        logger_close();
        exit(1);
    }

    // main thread break here.
    magt_loop(&myconfig);
    magt_close();

    time_t end_time = time(NULL);
    LOG_INFO ("END at %s\tprogram is totally running time of seconds %f",
              asctime(localtime( &end_time )), difftime(end_time, myapp.run_time));
    logger_close();

    return 0;
}
