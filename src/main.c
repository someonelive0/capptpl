
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include "magt.h"
#include "inputer.h"


int main(int argc, char** argv)
{
    pthread_t tid_magt, tid_inputer;
    int http_port = 3000;
    int input_port = 3001;

    if (0 != inputer_init(input_port)) {
        exit(1);
    }

    pthread_create(&tid_magt, NULL, magt, ((void *)&http_port));
    printf("start thread magt with tid %lld\n", tid_magt);

    pthread_create(&tid_inputer, NULL, inputer, ((void *)&input_port));
    printf("start thread inputer with tid %lld\n", tid_inputer);

    pthread_join(tid_magt, NULL);
    printf("join thread magt with tid %lld\n", tid_magt);

    inputer_stop();
    pthread_join(tid_inputer, NULL);
    printf("join thread inputer with tid %lld\n", tid_inputer);

    exit(0);
}
