#include <event.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/http_compat.h>

#include "logger.h"
#include "uthash.h"

#include "apptpl2_init.h"
#include "magt.h"


struct api_route {
    char path[32];        /* key */
    void (*cb)(struct evhttp_request *, void *); // evhttp callback
    UT_hash_handle hh;    /* makes this structure hashable */
};

struct api_route *routes = NULL;    /* important! initialize to NULL */
static void status_handler(struct evhttp_request *req, void *arg);
static void stats_handler(struct evhttp_request *req, void *arg);


int api_route_init() {
    struct api_route *r;

    r = malloc(sizeof *r);
    strcpy(r->path, "/");
    r->cb = status_handler;
    HASH_ADD_STR(routes, path, r);  /* id: name of key field */

    r = malloc(sizeof *r);
    strcpy(r->path, "/status");
    r->cb = status_handler;
    HASH_ADD_STR(routes, path, r);  /* id: name of key field */

    r = malloc(sizeof *r);
    strcpy(r->path, "/stats");
    r->cb = stats_handler;
    HASH_ADD_STR(routes, path, r);  /* id: name of key field */

    return 0;
}

void api_route_free() {
    struct api_route *current_route, *tmp;

    if (routes) {
        HASH_ITER(hh, routes, current_route, tmp) {
            HASH_DEL(routes, current_route);  /* delete; users advances to next */
            free(current_route);             /* optional- if you want to free  */
        }
    }
}


void api_handler(struct evhttp_request *req, void *arg) {
    struct evhttp_uri *decoded = NULL;
    char* decoded_path = NULL;

    enum evhttp_cmd_type cmd = evhttp_request_get_command(req);
    if (cmd != EVHTTP_REQ_GET && cmd != EVHTTP_REQ_HEAD) {
            return;
    }
    LOG_DEBUG ("evhttp_cmd_type %d", cmd);

    /* Decode the URI */
    decoded = evhttp_uri_parse(evhttp_request_get_uri(req));
    if (!decoded) {
            evhttp_send_error(req, HTTP_BADREQUEST, 0);
            return;
    }

    /* Let's see what path the user asked for. */
    const char *path = evhttp_uri_get_path(decoded);
    if (!path)
            path = "/";

    /* We need to decode it, to see what path the user really wanted. */
    decoded_path = evhttp_uridecode(path, 0, NULL);
    if (decoded_path == NULL)
            goto err;
    LOG_DEBUG ("decoded_path %s", decoded_path);

    struct api_route *r;
    HASH_FIND_STR(routes, decoded_path, r);
    if (r) {
        printf("route is %s, %p\n", r->path, r->cb);
        r->cb(req, arg);
        goto done;
    }

err:
    evhttp_send_error(req, HTTP_NOTFOUND, NULL);
done:
    if (decoded)
            evhttp_uri_free(decoded);
    if (decoded_path)
            free(decoded_path);
}

#define UNUSED(x) (void)(x)
static void status_handler(struct evhttp_request *req, void *arg) {
    UNUSED(arg);
    enum evhttp_cmd_type cmd = evhttp_request_get_command(req);
    LOG_DEBUG ("status_handler %d", cmd);

    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");
    //输出的内容
    struct evbuffer *buf;
    buf = evbuffer_new();
    evbuffer_add_printf(buf, "It works! status is ok\n%d\n", cmd);
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}

static void stats_handler(struct evhttp_request *req, void *arg) {
    UNUSED(arg);
    enum evhttp_cmd_type cmd = evhttp_request_get_command(req);
    LOG_DEBUG ("stats_handler %d", cmd);

    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");
    //输出的内容
    struct evbuffer *buf;
    buf = evbuffer_new();
    evbuffer_add_printf(buf, "It works! stats is statistic of program\n%d\n", cmd);
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}
