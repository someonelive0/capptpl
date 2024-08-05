/*
 * main.c for apptpl2
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "cchan_pthread.h"
#include "logger.h"
#include "init_log.h"
#include "load_config.h"

#include "apptpl2_init.h"
#include "magt.h"
#include "doredis.h"


int main(int argc, char** argv)
{
    time_t begin_time = time(NULL);
    int debug = 0;
    const char *config_filename = NULL;
    if (0 != parse_args(argc, (const char**)argv, &debug, &config_filename)) {
        exit(1);
    }
    if (NULL == config_filename) config_filename = DEFAULT_CONFIG_FILE;

    if (0 != init_log("apptpl2.log", debug, 1024*1024, 5)) {
        exit(1);
    }

    struct config myconfig;
    memset(&myconfig, 0, sizeof(struct config));
    if (load_config_ini(config_filename, ini_callback, &myconfig) < 0) {
        exit(1);
    }
    UT_string* s = config2json(&myconfig);
    LOG_INFO ("BEGIN at %s\tconfig_filename=%s\t%s",
              asctime(localtime( &begin_time )), // ctime(&begin_time),
              config_filename, utstring_body(s));
    utstring_free(s);

    // pthread_t tid_magt;
    cchan_t *chan_msg = cchan_new(sizeof(void*));    /* producers -> consumers */

    if (-1 == magt_init(&myconfig)) {
        LOG_ERROR ("msgt_init failed");
        exit(1);
    }
    redis_connect(myconfig.redis_host, myconfig.redis_port, myconfig.redis_passwd, magt_evbase);

    // main thread broke here.
    magt_loop(&myconfig);

    redis_close();
    magt_close();

    // pthread_create(&tid_magt, NULL, magt, ((void *)&myconfig));
    // LOG_INFO ("start thread magt with tid %lld", tid_magt);

    // brok here to wait
    // pthread_join(tid_magt, NULL);
    // LOG_INFO ("join thread magt with tid %lld", tid_magt);

    cchan_free(chan_msg);

    time_t end_time = time(NULL);
    LOG_INFO ("END at %s\tprogram is totally running time of seconds %f",
              asctime(localtime( &end_time )), difftime(end_time, begin_time));
    logger_close();

    // restart program itself.
    //restart(argv);

    // not reach here.
    exit(0);
}
