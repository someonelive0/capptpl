
#include <pthread.h>
#include <stdint.h>
#include "magt.h"


int main(int argc, char** argv)
{
    pthread_t tid_magt;
    int64_t port = 3000;

    pthread_create(&tid_magt, NULL, magt, ((void *)port));

    pthread_join(tid_magt, NULL);
    
    return 0;
}