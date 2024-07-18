
/*
 * management of http services.
 */
#include <stdlib.h>
#include <signal.h>
#include <event.h>
#include <event2/http_compat.h>

#include "logger.h"

#include "doredis.h"
#include "magt.h"

struct event_base *magt_evbase;
static struct evhttp *magt_httpd;
static struct event *magt_sigint, *magt_sigterm;
static struct event *magt_timer;

// static void httpd_handler(struct evhttp_request *req, void *arg);
#ifdef _WIN32
static void signal_cb(long long fd, short event, void *arg);
static void timer_cb(long long fd, short event, void *arg);
#else
static void signal_cb(int fd, short event, void *arg);
static void timer_cb(int fd, short event, void *arg);
#endif


int magt_init(struct config* myconfig) {
// link with -lwsock32
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(0x0201, &wsa_data);
#endif

    struct event_base* base = event_base_new();
    if (!base) {
        LOG_ERROR ("create event_base failed!");
        return -1;
    }

    // struct event evsigint;
    // event_set(&evsigint, SIGINT, EV_SIGNAL|EV_PERSIST, signal_cb, &evsignal);
    // event_add(&evsigint, NULL);
    struct event *evsigint = evsignal_new(base, SIGINT, signal_cb, base);
    evsignal_add(evsigint, 0);
    struct event *evsigterm = evsignal_new(base, SIGTERM, signal_cb, base);
    evsignal_add(evsigterm, 0);

    // struct event *evtimer = evtimer_new(base, timer_cb, base); // only call timer_cb once
    struct event *evtimer = event_new(base, -1, EV_TIMEOUT|EV_PERSIST, timer_cb, base);
    struct timeval timeout = {2, 0};
    evtimer_add(evtimer, &timeout); //  event_add(evtimer, 0);

    // add httpd event
    struct evhttp* httpd = evhttp_new(base);
    if (!httpd) {
        LOG_ERROR ("create evhttp failed!");
        return -1;
    }

    if (evhttp_bind_socket(httpd, "0.0.0.0", myconfig->http_port) != 0) {
        LOG_ERROR ("bind socket failed! port:%d", myconfig->http_port);
        return -1;
    }

    api_route_init();
    evhttp_set_gencb(httpd, api_handler, NULL);

    magt_evbase = base;
    magt_httpd = httpd;
    magt_sigint = evsigint;
    magt_sigterm = evsigterm;
    magt_timer = evtimer;
    return 0;
}

int magt_close() {
    evsignal_del(magt_sigint);
    evsignal_del(magt_sigterm);
    event_free(magt_sigint);
    event_free(magt_sigterm);
    event_free(magt_timer);
    evhttp_free(magt_httpd);
    event_base_free(magt_evbase);
    api_route_free();

    return 0;
}

void* magt_loop(struct config* myconfig) {
    LOG_INFO ("http listen port %d, start event loop", myconfig->http_port);
    event_base_dispatch(magt_evbase);
    LOG_INFO ("END event loop");

    return 0;
}

#if 0
// management interface thread
// param *data is struct config
void* magt(void *arg) {
    struct config* myconfig = (struct config*)arg;

// link with -lwsock32
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(0x0201, &wsa_data);
#endif

    struct event_base* base = event_base_new();
    if (!base) {
        LOG_ERROR ("create event_base failed!");
        return ((void*)1);
    }

    // struct event evsignal;
    // event_set(&evsignal, SIGINT, EV_SIGNAL|EV_PERSIST, signal_cb, &evsignal);
    // event_add(&evsignal, NULL);
    struct event *evsignal = evsignal_new(base, SIGINT, signal_cb, base);
    evsignal_add(evsignal, 0); //  event_add(evsignal, 0);

    // struct event *evtimer = evtimer_new(base, timer_cb, base); // only call timer_cb once
    struct event *evtimer = event_new(base, -1, EV_TIMEOUT|EV_PERSIST, timer_cb, base);
    struct timeval timeout = {2, 0};
    evtimer_add(evtimer, &timeout); //  event_add(evtimer, 0);

    // add httpd event
    struct evhttp* httpd = evhttp_new(base);
    if (!httpd) {
        LOG_ERROR ("create evhttp failed!");
        return ((void*)1);
    }

    if (evhttp_bind_socket(httpd, "0.0.0.0", myconfig->http_port) != 0) {
        LOG_ERROR ("bind socket failed! port:%d", myconfig->http_port);
        return ((void*)1);
    }

    evhttp_set_gencb(httpd, httpd_handler, NULL);

    redis_connect(myconfig->redis_host, myconfig->redis_port, myconfig->redis_passwd, base);

    LOG_INFO ("http listen port %d, start event loop", myconfig->http_port);
    event_base_dispatch(base);
    LOG_INFO ("END event loop");

    redis_close();

    evhttp_free(httpd);
    evsignal_del(evsignal);
    event_free(evsignal);
    event_free(evtimer);
    event_base_free(base);

    return ((void*)0);
}

static void httpd_handler(struct evhttp_request *req, void *arg) {
    UNUSED(arg);
    char output[2048] = "\0";
    char tmp[1024];

    //获取客户端请求的URI(使用evhttp_request_uri或直接req->uri)
    const char *uri;
    uri = evhttp_request_uri(req);
    LOG_DEBUG ("http request uri=%s", uri);
    snprintf(tmp, sizeof(tmp)-1, "uri=%s\n", uri);
    strncat(output, tmp, sizeof(tmp)-1);

    snprintf(tmp, sizeof(tmp)-1, "uri=%s\n", req->uri);
    strncat(output, tmp, sizeof(tmp)-1);
    //decoded uri
    char *decoded_uri;
    decoded_uri = evhttp_decode_uri(uri);
    snprintf(tmp, sizeof(tmp)-1, "decoded_uri=%s", decoded_uri);
    strncat(output, tmp, sizeof(tmp)-1);

    //解析URI的参数(即GET方法的参数)
    struct evkeyvalq params;
    //将URL数据封装成key-value格式,q=value1, s=value2
    evhttp_parse_query(decoded_uri, &params);
    //得到q所对应的value
    snprintf(tmp, sizeof(tmp)-1, "q=%s\n", evhttp_find_header(&params, "q"));
    strncat(output, tmp, sizeof(tmp)-1);
    //得到s所对应的value
    snprintf(tmp, sizeof(tmp)-1, "s=%s\n", evhttp_find_header(&params, "s"));
    strncat(output, tmp, sizeof(tmp)-1);

    free(decoded_uri);

    //获取POST方法的数据
    char *post_data = (char *) EVBUFFER_DATA(req->input_buffer);
    snprintf(tmp, sizeof(tmp)-1, "post_data=%s\n", post_data);
    strncat(output, tmp, sizeof(tmp)-1);

    /*
       具体的：可以根据GET/POST的参数执行相应操作，然后将结果输出
       ...
     */

    /* 输出到客户端 */

    //HTTP header
    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");
    //输出的内容
    struct evbuffer *buf;
    buf = evbuffer_new();
    evbuffer_add_printf(buf, "It works!\n%s\n", output);
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}
#endif

#define UNUSED(x) (void)(x)
#ifdef _WIN32
static void signal_cb(long long fd, short event, void *arg) {
#else
static void signal_cb(int fd, short event, void *arg) {
#endif
    if (event == EV_SIGNAL) { // should be EV_SIGNAL=8
        LOG_INFO ("%s: got signal %d", __func__, fd);
        if (fd == SIGINT || fd == SIGTERM)
            event_base_loopbreak(arg);  // arg = evbase
    }
}

#ifdef _WIN32
static void timer_cb(long long fd, short event, void *arg) {
#else
static void timer_cb(int fd, short event, void *arg) {
#endif
    UNUSED(fd);
    UNUSED(event);
    UNUSED(arg);
    // LOG_TRACE ("%s: got timeout with unix time: %lld\n", __func__, time(NULL));
    logger_flush();
}
