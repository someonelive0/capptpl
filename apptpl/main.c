/*
 * main.c for apptpl
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "cchan_pthread.h"
#include "logger.h"

#include "apptpl_init.h"
#include "init_log.h"
#include "load_config.h"
#include "magt.h"
#include "inputer.h"
#include "worker.h"
#include "capture.h"


int main(int argc, char** argv)
{
    time_t begin_time = time(NULL);
    int debug = 0;
    const char *config_filename = NULL; 
    if (0 != parse_args(argc, (const char**)argv, &debug, &config_filename)) {
        exit(1);
    }
    if (NULL == config_filename) config_filename = DEFAULT_CONFIG_FILE;

    if (0 != init_log("apptpl.log", debug, 1024*1024, 5)) {
        exit(1);
    }

    struct config myconfig = {{0}, 0, 0, {0}, 0, 0, {0}};
    if (load_config_ini(config_filename, ini_callback, &myconfig) < 0) {
        exit(1);
    }
    LOG_INFO ("BEGIN at %s\tmyconfig=%s, version=%s, http_port=%d, zmq_port=%d, pcap_device=%s, pcap_snaplen=%d, pcap_buffer_size=%d, pcap_filter=%s",
        asctime(localtime( &begin_time )), // ctime(&begin_time),
        config_filename, myconfig.version, myconfig.http_port, myconfig.zmq_port,
        myconfig.pcap_device, myconfig.pcap_snaplen, myconfig.pcap_buffer_size, myconfig.pcap_filter);

    // init something
    if (-1 == magt_init(&myconfig)) {
        LOG_ERROR ("msgt_init failed");
        exit(1);
    }

    if (0 != inputer_init(myconfig.zmq_port)) {
        exit(1);
    }

    // don't use mingw64 libpcap-devel, use npcap-sdk-1.13 to link -lwpcap.
    if (0 != capture_open_device(myconfig.pcap_device,
        myconfig.pcap_snaplen, myconfig.pcap_buffer_size, myconfig.pcap_filter)) {
        exit(1);
    }

    pthread_t tid_inputer, tid_worker, tid_capture;
    cchan_t *chan_msg = cchan_new(sizeof(void*));    /* producers -> consumers */

    // pthread_create(&tid_magt, NULL, magt, ((void *)&myconfig));
    // LOG_INFO ("start thread magt with tid %lld", tid_magt);

    pthread_create(&tid_worker, NULL, worker, ((void *)chan_msg));
    LOG_INFO ("start thread worker with tid %lld", tid_worker);

    pthread_create(&tid_inputer, NULL, inputer, ((void *)chan_msg));
    LOG_INFO ("start thread inputer with tid %lld", tid_inputer);

    pthread_create(&tid_capture, NULL, capture, ((void *)chan_msg));
    LOG_INFO ("start thread capture with tid %lld", tid_capture);

    // broke here to wait
    // pthread_join(tid_magt, NULL);
    // LOG_INFO ("join thread magt with tid %lld", tid_magt);

    // main thread broke here.
    magt_loop(&myconfig);
    magt_close();

    inputer_stop();
    pthread_join(tid_inputer, NULL);
    LOG_INFO ("join thread inputer with tid %lld", tid_inputer);

    capture_shutdown = 1;
    pthread_join(tid_capture, NULL);
    LOG_INFO ("join thread capture with tid %lld", tid_capture);
    capture_close();

    worker_shutdown = 1;
    pthread_join(tid_worker, NULL);
    LOG_INFO ("join thread worker with tid %lld", tid_worker);

    cchan_free(chan_msg);

    time_t end_time = time(NULL);
    LOG_INFO ("END at %s\tprogram is totally running time of seconds %f",
        asctime(localtime( &end_time )), difftime(end_time, begin_time));
    logger_close();
    exit(0);
}
