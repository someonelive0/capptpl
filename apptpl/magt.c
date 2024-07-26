
/*
 * management of http services.
 */
#include <stdlib.h>
#include <signal.h>
#include <event.h>
#include <event2/http_compat.h>

#include "logger.h"

#include "magt.h"
#include "parser.h"

#define TIMER_INTERVAL 2  // timer interval in seconds


struct event_base *magt_evbase;
static struct evhttp *magt_httpd;
static struct event *magt_sigint, *magt_sigterm;
static struct event *magt_timer;

#ifdef _WIN32
static void signal_cb(long long fd, short event, void *arg);
static void timer_cb(long long fd, short event, void *arg);
#else
static void signal_cb(int fd, short event, void *arg);
static void timer_cb(int fd, short event, void *arg);
#endif


int magt_init(struct app* myapp)
{
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
    struct event *evtimer = event_new(base, -1, EV_TIMEOUT|EV_PERSIST, timer_cb, myapp);
    struct timeval timeout = {TIMER_INTERVAL, 0};
    evtimer_add(evtimer, &timeout); //  event_add(evtimer, 0);

    // add httpd event
    struct evhttp* httpd = evhttp_new(base);
    if (!httpd) {
        LOG_ERROR ("create evhttp failed!");
        return -1;
    }

    if (evhttp_bind_socket(httpd, "0.0.0.0", myapp->myconfig->http_port) != 0) {
        LOG_ERROR ("bind socket failed! port:%d", myapp->myconfig->http_port);
        return -1;
    }

    if (-1 == api_route_init())
        return -1;
    evhttp_set_gencb(httpd, api_handler, myapp);

    magt_evbase = base;
    magt_httpd = httpd;
    magt_sigint = evsigint;
    magt_sigterm = evsigterm;
    magt_timer = evtimer;
    return 0;
}

int magt_close()
{
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

void* magt_loop(struct config* myconfig)
{
    LOG_INFO ("http listen port %d, start event loop", myconfig->http_port);
    event_base_dispatch(magt_evbase);
    LOG_INFO ("END event loop");

    return 0;
}

#define UNUSED(x) (void)(x)
#ifdef _WIN32
static void signal_cb(long long fd, short event, void *arg)
#else
static void signal_cb(int fd, short event, void *arg)
#endif
{
    if (event == EV_SIGNAL) { // should be EV_SIGNAL=8
        LOG_INFO ("%s: got signal %d", __func__, fd);
        if (fd == SIGINT || fd == SIGTERM)
            event_base_loopbreak(arg);  // arg = evbase
    }
}

#ifdef _WIN32
static void timer_cb(long long fd, short event, void *arg)
#else
static void timer_cb(int fd, short event, void *arg)
#endif
{
    UNUSED(fd);     // always -1
    UNUSED(event);  // always 1
    struct app* myapp = arg;
    parser_time_ev(myapp->prsr, TIMER_INTERVAL);
    // LOG_TRACE ("%s: got timeout with unix time: %lld\n", __func__, time(NULL));
    logger_flush();
}
