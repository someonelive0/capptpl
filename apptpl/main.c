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
#include "parser.h"


int main(int argc, char** argv)
{
    if (0 != ch_exec_cwd(argv[0]))
        exit(1);

    struct app myapp = { time(NULL), NULL, NULL, NULL, NULL, NULL };
    int debug = 0;
    const char *config_filename = NULL;
    if (0 != parse_args(argc, (const char**)argv, &debug, &config_filename)) {
        exit(1);
    }
    if (NULL == config_filename) config_filename = DEFAULT_CONFIG_FILE;

    if (0 != init_log("apptpl.log", debug, 1024*1024, 5)) {
        exit(1);
    }

    struct config myconfig;
    memset(&myconfig, 0, sizeof(struct config));
    if (load_config_ini(config_filename, ini_callback, &myconfig) < 0) {
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
        LOG_ERROR ("msgt_init failed");
        exit(1);
    }

    cchan_t *chan_msg = cchan_new(sizeof(void*));    // channel of zmq messages
    cchan_t *chan_pkt = cchan_new(sizeof(void*));    // channel of pcap pkts
    struct worker wrkr = {0, chan_msg, 0};
    struct inputer inptr;
    memset(&inptr, 0, sizeof(struct inputer));
    if (0 != inputer_open(&inptr, myconfig.zmq_port, chan_msg)) {
        goto err;
    }
    myapp.inptr = &inptr;
    myapp.wrkr = & wrkr;

    struct parser prsr = {0, chan_pkt, 0, 0, 0};
    struct capture captr;
    memset(&captr, 0, sizeof(struct capture));
    captr.chan_pkt = chan_pkt;
    if (0 != capture_open(&captr, myconfig.pcap_device, myconfig.pcap_snaplen,
                          myconfig.pcap_buffer_size, myconfig.pcap_filter)) {
        goto err;
    }
    myapp.captr = &captr;
    myapp.prsr = & prsr;

    pthread_t tid_inputer, tid_worker, tid_capture, tid_parser;

    // pthread_create(&tid_magt, NULL, magt, ((void *)&myconfig));
    // LOG_INFO ("start thread magt with tid %lld", tid_magt);

    pthread_create(&tid_worker, NULL, worker_loop, ((void *)&wrkr));
    LOG_INFO ("start thread worker with tid %lld", tid_worker);

    pthread_create(&tid_inputer, NULL, inputer_loop, ((void *)&inptr));
    LOG_INFO ("start thread inputer with tid %lld", tid_inputer);

    pthread_create(&tid_parser, NULL, parser_loop, ((void *)&prsr));
    LOG_INFO ("start thread parser with tid %lld", tid_parser);

    pthread_create(&tid_capture, NULL, capture_loop, ((void *)&captr));
    LOG_INFO ("start thread capture with tid %lld", tid_capture);

    // broke here to wait
    // pthread_join(tid_magt, NULL);
    // LOG_INFO ("join thread magt with tid %lld", tid_magt);

    // main thread broke here.
    magt_loop(&myconfig);
    magt_close();

    inputer_stop(&inptr);
    pthread_join(tid_inputer, NULL);
    LOG_DEBUG ("join thread inputer with tid %lld", tid_inputer);

    // capture_shutdown = 1;
    captr.shutdown = 1;
    pthread_join(tid_capture, NULL);
    LOG_DEBUG ("join thread capture with tid %lld", tid_capture);
    // capture_close_device();
    capture_close(&captr);

    wrkr.shutdown = 1;
    pthread_join(tid_worker, NULL);
    LOG_DEBUG ("join thread worker with tid %lld", tid_worker);

    prsr.shutdown = 1;
    pthread_join(tid_parser, NULL);
    LOG_DEBUG ("join thread parser with tid %lld", tid_parser);

    cchan_free(chan_msg);
    cchan_free(chan_pkt);

    time_t end_time = time(NULL);
    LOG_INFO ("END at %s\tprogram is totally running time of seconds %f",
              asctime(localtime( &end_time )), difftime(end_time, myapp.run_time));
    logger_close();
    exit(0);

err:
    magt_close();
    inputer_stop(&inptr);
    capture_close(&captr);
    if (chan_msg) cchan_free(chan_msg);
    if (chan_pkt) cchan_free(chan_pkt);
    logger_close();
    exit(1);
}
