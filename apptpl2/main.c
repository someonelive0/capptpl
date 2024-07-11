/*
 * main.c for apptpl2
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "cchan_pthread.h"
#include "logger.h"

#include "version.h"
#include "init_log.h"
#include "magt.h"


int main(int argc, char** argv)
{
    time_t begin_time = time(NULL);
    int debug = 0;
    const char *config_filename = NULL; 
    // if (0 != parse_args(argc, (const char**)argv, &debug, &config_filename)) {
    //     exit(1);
    // }
    // if (NULL == config_filename) config_filename = DEFAULT_CONFIG_FILE;

    if (0 != init_log("apptpl2.log", debug, 1024*1024, 5)) {
        exit(1);
    }

    // struct config myconfig = {{0}, 0, 0, {0}};
    // if (load_config(config_filename, ini_callback, &myconfig) < 0) {
    //     exit(1);
    // }
    // LOG_INFO ("BEGIN at %s\tmyconfig=%s, version=%s, http_port=%d, zmq_port=%d, pcap_device=%s",
    //     asctime(localtime( &begin_time )), // ctime(&begin_time),
    //     config_filename, myconfig.version, myconfig.http_port, myconfig.zmq_port, myconfig.pcap_device);


    pthread_t tid_magt, tid_inputer, tid_worker, tid_capture;
    cchan_t *chan_msg = cchan_new(sizeof(void*));    /* producers -> consumers */

    int http_port = 3000;
    pthread_create(&tid_magt, NULL, magt, ((void *)&http_port));
    LOG_INFO ("start thread magt with tid %lld", tid_magt);

    // brok here to wait
    pthread_join(tid_magt, NULL);
    LOG_INFO ("join thread magt with tid %lld", tid_magt);

    cchan_free(chan_msg);

    time_t end_time = time(NULL);
    LOG_INFO ("END at %s\tprogram is totally running time of seconds %f",
        asctime(localtime( &end_time )), difftime(end_time, begin_time));
    exit(0);
}
