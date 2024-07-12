#include <zmq.h>
#include <unistd.h>
#include <stdlib.h>

#include "ini.h"
#include "logger.h"


static int init_log();
static int ini_cb(void* arg, const char* section, const char* name, const char* value);
static int copy_file(const char* in_path, const char* out_path);
static int load_config(const char* filename);

#define UNUSED(x) (void)(x)
int main(int argc, char** argv) {
    UNUSED(argc);
    UNUSED(argv);
    char addr[24] = {0};
    int port = 3001;
    int rc, count = 0;

    if (load_config("sndmsg.ini") < 0) {
        exit(1);
    }

    init_log();

    void *context = zmq_ctx_new();
    void *pusher = zmq_socket (context, ZMQ_PUSH);
    snprintf(addr, sizeof(addr)-1, "tcp://localhost:%d", port);
    if(zmq_connect(pusher, addr) == -1) {
        printf("E: connect %s failed: %s\n", addr,  zmq_strerror(zmq_errno()));
        exit(1);
    }

    zmq_msg_t msg;
    for (int i=0; i<1000000; i++) {
        if (0 != zmq_msg_init_size (&msg, strlen(addr))) {
            printf("zmq zmq_msg_init failed: %d, %s\n", zmq_errno(), zmq_strerror(zmq_errno()));
        }
        memcpy (zmq_msg_data (&msg), addr, strlen(addr));
        rc = zmq_msg_send (&msg, pusher, 0);
        // printf("zmq_msg_send len %d\n", rc);
        if (rc != (int)strlen(addr)) {
            printf("zmq_msg_send len %d not equle data len %zd\n", rc, strlen(addr));
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

static int init_log() {
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

static int ini_cb(void* arg, const char* section, const char* name, const char* value) {
    UNUSED(arg);
    // configuration* pconfig = (configuration*)arg;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("global", "version")) {
        printf("ini global %s, name %s, value %s\n", section, name, value);
        // pconfig->version = atoi(value);
    } else if (MATCH("user", "name")) {
        // pconfig->name = strdup(value);
    } else if (MATCH("user", "email")) {
        // pconfig->email = strdup(value);
    } else {
        printf("ini section %s, name %s, value %s\n", section, name, value);
        // return 0;  /* unknown section/name, error */
    }
    return 1;
}

static int copy_file(const char* in_path, const char* out_path) {
    size_t n;
    FILE* in=NULL, * out=NULL;
    char buf[64];
    
    if((in = fopen(in_path, "rb")) && (out = fopen(out_path, "wb")))
        while((n = fread(buf, 1, sizeof(buf), in)) && fwrite(buf, 1, n, out));
    else return -1;

    if(in) fclose(in);
    if(out) fclose(out);
    return 0;
}

static int load_config(const char* filename) {
    char* tpl_filename;
    int rc;

    rc = access(filename, F_OK);
    if (0 != rc) {
        printf("ini file '%s' not exists\n", filename);
        tpl_filename = malloc(strlen(filename) + 5);
        strcpy(tpl_filename, filename);
        strcat(tpl_filename, ".tpl");
        rc = copy_file(tpl_filename, filename);
        free(tpl_filename);
        if (0 != rc) {
            printf("copy ini tpl file '%s.tpl' to ini '%s' failed\n", filename, filename);
            return -1;
        }
    }

    if (ini_parse(filename, ini_cb, NULL) < 0) {
        printf("parse ini file failed '%s'\n", filename);
        return -1;
    }

    return 0;
}
