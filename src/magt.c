
// #ifdef _WIN32
// pragma comment(lib, "ws2_32.lib")
// #endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     //for getopt, fork
#include <string.h>     //for strcat
//for struct evkeyvalq
// #include <sys/queue.h>
//#undef EVENT__HAVE_NETDB_H
#include <event.h>
//for http
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/http_compat.h>
// #include <event2/util.h>
// #include <event2/timer.h>
#include <signal.h>

#include "magt.h"


void httpd_handler(struct evhttp_request *req, void *arg) {
    char output[2048] = "\0";
    char tmp[1024];

    //获取客户端请求的URI(使用evhttp_request_uri或直接req->uri)
    const char *uri;
    uri = evhttp_request_uri(req);
    sprintf(tmp, "uri=%s\n", uri);
    strcat(output, tmp);

    sprintf(tmp, "uri=%s\n", req->uri);
    strcat(output, tmp);
    //decoded uri
    char *decoded_uri;
    decoded_uri = evhttp_decode_uri(uri);
    sprintf(tmp, "decoded_uri=%s\n", decoded_uri);
    strcat(output, tmp);

    //解析URI的参数(即GET方法的参数)
    struct evkeyvalq params;
    //将URL数据封装成key-value格式,q=value1, s=value2
    evhttp_parse_query(decoded_uri, &params);
    //得到q所对应的value
    sprintf(tmp, "q=%s\n", evhttp_find_header(&params, "q"));
    strcat(output, tmp);
    //得到s所对应的value
    sprintf(tmp, "s=%s\n", evhttp_find_header(&params, "s"));
    strcat(output, tmp);

    free(decoded_uri);

    //获取POST方法的数据
    char *post_data = (char *) EVBUFFER_DATA(req->input_buffer);
    sprintf(tmp, "post_data=%s\n", post_data);
    strcat(output, tmp);

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

int called = 0;
static void signall_cb(long long fd, short event, void *arg)
{
    struct event *signal = arg;
    printf("%s: got signal %d\n", __func__, EVENT_SIGNAL(signal));
    event_base_loopbreak(arg);  //终止侦听event_dispatch()的事件侦听循环，执行之后的代码
    if (called >= 2)
        evsignal_del(signal);
     called++;
}

static void timer_cb(long long fd, short event, void *arg) {
    printf("%s: got timeout %ld\n", __func__, time(NULL));
}


// management interface thread
// param *data is int port
void* magt(void *arg) {

    int64_t port = ((int64_t)arg);

// link with -lwsock32
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(0x0201, &wsa_data);
#endif

    struct event_base* base = event_base_new();
    if (!base)
    {
        printf("create event_base failed!\n");
        return ((void*)1);
    }

    // struct event signal_int;
    // event_set(&signal_int, SIGINT, EV_SIGNAL|EV_PERSIST, signall_cb, &signal_int);
    // event_add(&signal_int, NULL);
    struct event *signal_int = evsignal_new(base, SIGINT, signall_cb, base);
    evsignal_add(signal_int, 0); //  event_add(signal_int, 0);

    // struct event *evtimer = evtimer_new(base, timer_cb, base); // only call timer_cb once
    struct event *evtimer = event_new(base, -1, EV_TIMEOUT|EV_PERSIST, timer_cb, base);
    struct timeval timeout = {2, 0};
    evtimer_add(evtimer, &timeout); //  event_add(signal_int, 0);

    // add httpd event
    struct evhttp* httpd = evhttp_new(base);
    if (!httpd)
    {
        printf("create evhttp failed!\n");
        return ((void*)1);
    }

    if (evhttp_bind_socket(httpd, "0.0.0.0", port) != 0)
    {
        printf("bind socket failed! port:%d\n", port);
        return ((void*)1);
    }

    evhttp_set_gencb(httpd, httpd_handler, NULL);

    event_base_dispatch(base);
    printf("END evloop\n");

    evhttp_free(httpd);
    evsignal_del(signal_int);
    event_free(signal_int);
    event_free(evtimer);
    event_base_free(base);

    return ((void*)0);
}
