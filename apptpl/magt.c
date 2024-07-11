
/*
 * management of http services.
 */
#include <stdlib.h>
#include <signal.h>
#include <event.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/http_compat.h>

#include "logger.h"

#include "magt.h"


#define UNUSED(x) (void)(x)
void httpd_handler(struct evhttp_request *req, void *arg) {
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

int signal_called = 0;
#ifdef _WIN32
static void signal_cb(long long fd, short event, void *arg) {
#else
static void signal_cb(int fd, short event, void *arg) {
#endif
    UNUSED(fd);
    UNUSED(event);
    struct event *signal = arg;
    LOG_INFO ("%s: got signal %d", __func__, EVENT_SIGNAL(signal));
    event_base_loopbreak(arg);  //终止侦听event_dispatch()的事件侦听循环，执行之后的代码
    if (signal_called >= 2)
        evsignal_del(signal);
     signal_called ++;
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
}


// management interface thread
// param *data is int port
void* magt(void *arg) {

    int port = (*(int*)arg);

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

    // struct event signal_int;
    // event_set(&signal_int, SIGINT, EV_SIGNAL|EV_PERSIST, signal_cb, &signal_int);
    // event_add(&signal_int, NULL);
    struct event *signal_int = evsignal_new(base, SIGINT, signal_cb, base);
    evsignal_add(signal_int, 0); //  event_add(signal_int, 0);

    // struct event *evtimer = evtimer_new(base, timer_cb, base); // only call timer_cb once
    struct event *evtimer = event_new(base, -1, EV_TIMEOUT|EV_PERSIST, timer_cb, base);
    struct timeval timeout = {2, 0};
    evtimer_add(evtimer, &timeout); //  event_add(signal_int, 0);

    // add httpd event
    struct evhttp* httpd = evhttp_new(base);
    if (!httpd) {
        LOG_ERROR ("create evhttp failed!");
        return ((void*)1);
    }

    if (evhttp_bind_socket(httpd, "0.0.0.0", port) != 0) {
        LOG_ERROR ("bind socket failed! port:%d", port);
        return ((void*)1);
    }

    evhttp_set_gencb(httpd, httpd_handler, NULL);

    LOG_INFO ("http listen port %d, start event loop", port);
    event_base_dispatch(base);
    LOG_INFO ("END event loop");

    evhttp_free(httpd);
    evsignal_del(signal_int);
    event_free(signal_int);
    event_free(evtimer);
    event_base_free(base);

    return ((void*)0);
}
