
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "cchan_pthread.h"
#include "logger.h"

#include "init_log.h"
#include "magt.h"
#include "inputer.h"
#include "worker.h"


struct config {
    char version[8];
    int http_port;
    int zmq_port;
};

int ini_callback(void* arg, const char* section, const char* name, const char* value)
{
    struct config* pconfig = (struct config*)arg;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("global", "version")) {
        printf("ini global %s, name %s, value %s\n", section, name, value);
        strncpy(pconfig->version, value, sizeof(pconfig->version)-1);
    } else if (MATCH("http", "port")) {
        pconfig->http_port = atoi(value);
    } else if (MATCH("zmq", "port")) {
        pconfig->zmq_port = atoi(value);
    } else {
        printf("unknown ini section %s, name %s, value %s\n", section, name, value);
        return 0;  /* unknown section/name, error */
    }
    return 1;
}


int main(int argc, char** argv)
{
    pthread_t tid_magt, tid_inputer, tid_worker;
    // int http_port = 3000;
    // int input_port = 3001;
    cchan_t *chan_msg = cchan_new(sizeof(void*));    /* producers -> consumers */

    struct config myconfig = {{0}, 0, 0};
    if (load_config("apptpl.ini", ini_callback, &myconfig) < 0) {
        exit(1);
    }

    init_log("apptpl.log", 1024*1024, 5);

    if (0 != inputer_init(myconfig.zmq_port)) {
        exit(1);
    }

    pthread_create(&tid_magt, NULL, magt, ((void *)&myconfig.http_port));
    LOG_INFO ("start thread magt with tid %lld", tid_magt);

    pthread_create(&tid_worker, NULL, worker, ((void *)chan_msg));
    LOG_INFO ("start thread worker with tid %lld", tid_worker);

    pthread_create(&tid_inputer, NULL, inputer, ((void *)chan_msg));
    LOG_INFO ("start thread inputer with tid %lld", tid_inputer);

    pthread_join(tid_magt, NULL);
    LOG_INFO ("join thread magt with tid %lld", tid_magt);

    inputer_stop();
    pthread_join(tid_inputer, NULL);
    LOG_INFO ("join thread inputer with tid %lld", tid_inputer);

    worker_shutdown = 1;
    pthread_join(tid_worker, NULL);
    LOG_INFO ("join thread worker with tid %lld", tid_worker);

    cchan_free(chan_msg);

    exit(0);
}
