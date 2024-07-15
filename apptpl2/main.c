/*
 * main.c for apptpl2
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "cchan_pthread.h"
#include "logger.h"

#include "apptpl2_init.h"
#include "init_log.h"
#include "load_config.h"
#include "magt.h"


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

    struct config myconfig = {{0}, 0, {0}, 0, {0}};
    if (load_config_ini(config_filename, ini_callback, &myconfig) < 0) {
        exit(1);
    }
    LOG_INFO ("BEGIN at %s\tmyconfig=%s, version=%s, http_port=%d, redis_host=%s, redis_port=%d",
        asctime(localtime( &begin_time )), // ctime(&begin_time),
        config_filename, myconfig.version, myconfig.http_port, myconfig.redis_host, myconfig.redis_port);


    // pthread_t tid_magt;
    cchan_t *chan_msg = cchan_new(sizeof(void*));    /* producers -> consumers */

    if (-1 == magt_init(&myconfig)) {
        LOG_ERROR ("msgt_init failed");
        exit(1);
    }

    // main thread broke here.
    magt_loop(&myconfig);
    magt_close();

    // pthread_create(&tid_magt, NULL, magt, ((void *)&myconfig));
    // LOG_INFO ("start thread magt with tid %lld", tid_magt);

    // // brok here to wait
    // pthread_join(tid_magt, NULL);
    // LOG_INFO ("join thread magt with tid %lld", tid_magt);

    cchan_free(chan_msg);

    time_t end_time = time(NULL);
    LOG_INFO ("END at %s\tprogram is totally running time of seconds %f",
        asctime(localtime( &end_time )), difftime(end_time, begin_time));
    exit(0);
}
