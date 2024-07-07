
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>

#include "cchan_pthread.h"

#include "magt.h"
#include "inputer.h"
#include "worker.h"


int main(int argc, char** argv)
{
    pthread_t tid_magt, tid_inputer, tid_worker;
    int http_port = 3000;
    int input_port = 3001;
    cchan_t *chan_msg = cchan_new(sizeof(void*));    /* producers -> consumers */


    if (0 != inputer_init(input_port)) {
        exit(1);
    }

    pthread_create(&tid_magt, NULL, magt, ((void *)&http_port));
    printf("start thread magt with tid %lld\n", tid_magt);

    pthread_create(&tid_worker, NULL, worker, ((void *)chan_msg));
    printf("start thread worker with tid %lld\n", tid_worker);

    pthread_create(&tid_inputer, NULL, inputer, ((void *)chan_msg));
    printf("start thread inputer with tid %lld\n", tid_inputer);

    pthread_join(tid_magt, NULL);
    printf("join thread magt with tid %lld\n", tid_magt);

    inputer_stop();
    pthread_join(tid_inputer, NULL);
    printf("join thread inputer with tid %lld\n", tid_inputer);

    worker_shutdown = 1;
    pthread_join(tid_worker, NULL);
    printf("join thread worker with tid %lld\n", tid_worker);

    cchan_free(chan_msg);

    exit(0);
}
