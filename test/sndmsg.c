#include <zmq.h>
#include <unistd.h>
#include <stdlib.h>

#include "logger.h"


int init_log() {
    logger_initConsoleLogger(NULL);
    logger_initFileLogger("sndmsg.log", 1024*1024, 5);
    LOG_INFO("multi logging");
    logger_setLevel(LogLevel_DEBUG);
    // if (logger_isEnabled(LogLevel_DEBUG)) {
    //     for (int i = 0; i < 8; i++) {
    //         LOG_DEBUG("%d", i);
    //         sleep(1);
    //     }
    // }
    return 0 ;
}

int main(int argc, char** argv)
{
    char addr[24] = {0};
    int port = 3001;
    int rc, count = 0;

    init_log();

    void *context = zmq_ctx_new();
    void *pusher = zmq_socket (context, ZMQ_PUSH);
    snprintf(addr, sizeof(addr)-1, "tcp://localhost:%d", port);
    if(zmq_connect(pusher, addr) == -1) {
        printf("E: connect %s failed: %s\n", addr,  zmq_strerror(zmq_errno()));
        return -1;
    }

    zmq_msg_t msg;
    for (int i=0; i<1000000; i++) {
        if (0 != zmq_msg_init_size (&msg, strlen(addr))) {
            printf("zmq zmq_msg_init failed: %d, %s\n", zmq_errno(), zmq_strerror(zmq_errno()));
        }
        memcpy (zmq_msg_data (&msg), addr, strlen(addr));
        rc = zmq_msg_send (&msg, pusher, 0);
        // printf("zmq_msg_send len %d\n", rc);
        if (rc != strlen(addr)) {
            printf("zmq_msg_send len %d not equle data len %lld\n", rc, strlen(addr));
        }

        count ++;
        if ((count % 10000) == 0) {
            // printf("send msg count %d, len %lld: [%s]\n",
            //     count, zmq_msg_size (&msg), (char*)zmq_msg_data (&msg));
            LOG_DEBUG("send msg count %d, len %lld: [%s]",
                count, zmq_msg_size (&msg), (char*)zmq_msg_data (&msg));
        }

        if ((rc = zmq_msg_close(&msg)) != 0) {
            printf("zmq_msg_close msg failed: %d, %s\n", zmq_errno(), zmq_strerror(zmq_errno()));
        }
    }

    zmq_close (pusher);
    zmq_ctx_term (context);

    exit(0);
}
