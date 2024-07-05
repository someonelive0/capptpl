
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include "magt.h"


int main(int argc, char** argv)
{
    pthread_t tid_magt;
    int port = 3000;

    pthread_create(&tid_magt, NULL, magt, ((void *)&port));
    printf("start thread magt with tid %lld\n", tid_magt);

    pthread_join(tid_magt, NULL);
    printf("join thread magt with tid %lld\n", tid_magt);

    return 0;
}
