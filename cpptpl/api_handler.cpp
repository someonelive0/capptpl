#include "magt.h"

//#include <stdatomic.h>
#include <event.h>

#include "logger.h"
#include "uthash.h"
#include "cJSON.h"

#include "version.h"
// #include "capture.h"
// #include "parser.h"
// #include "inputer.h"
// #include "worker.h"


/*
 * hash table with key is string of path,
 * value is functin pointer to process the path of http.
 */
struct api_route {
    char path[32];        /* key */
    void (*cb)(struct evhttp_request *, void *); // evhttp callback
    UT_hash_handle hh;    /* makes this structure hashable */
};

struct api_route *routes = NULL;    /* important! initialize to NULL */
static void status_handler(struct evhttp_request *req, void *arg);
static void version_handler(struct evhttp_request *req, void *arg);
static void stats_handler(struct evhttp_request *req, void *arg);
static void config_handler(struct evhttp_request *req, void *arg);
static void dump_handler(struct evhttp_request *req, void *arg);


int api_route_init()
{
    struct api_route *r;

    if (NULL == (r = (api_route*)malloc(sizeof *r))) return -1;
    strcpy(r->path, "/");
    r->cb = status_handler;
    HASH_ADD_STR(routes, path, r);

    if (NULL == (r = (api_route*)malloc(sizeof *r))) return -1;
    strcpy(r->path, "/status");
    r->cb = status_handler;
    HASH_ADD_STR(routes, path, r);

    if (NULL == (r = (api_route*)malloc(sizeof *r))) return -1;
    strcpy(r->path, "/version");
    r->cb = version_handler;
    HASH_ADD_STR(routes, path, r);

    if (NULL == (r = (api_route*)malloc(sizeof *r))) return -1;
    strcpy(r->path, "/stats");
    r->cb = stats_handler;
    HASH_ADD_STR(routes, path, r);

    if (NULL == (r = (api_route*)malloc(sizeof *r))) return -1;
    strcpy(r->path, "/config");
    r->cb = config_handler;
    HASH_ADD_STR(routes, path, r);

    if (NULL == (r = (api_route*)malloc(sizeof *r))) return -1;
    strcpy(r->path, "/dump");
    r->cb = dump_handler;
    HASH_ADD_STR(routes, path, r);

    return 0;
}

void api_route_free()
{
    struct api_route *current_route, *tmp;

    if (routes) {
        HASH_ITER(hh, routes, current_route, tmp) {
            HASH_DEL(routes, current_route);  /* delete; users advances to next */
            free(current_route);             /* optional- if you want to free  */
        }
    }
}


/*
 * api_handler is event2/http callback function.
 */
void api_handler(struct evhttp_request *req, void *arg)
{
    struct evhttp_uri *decoded = NULL;
    char* decoded_path = NULL;

    enum evhttp_cmd_type cmd = evhttp_request_get_command(req);
    if (cmd != EVHTTP_REQ_GET && cmd != EVHTTP_REQ_HEAD) {
        return;
    }
    // LOG_DEBUG ("evhttp_cmd_type %d", cmd);

    struct evkeyvalq * headers = evhttp_request_get_input_headers (req);
    const char* api_key = evhttp_find_header(headers, "Api-key");
    if (api_key == NULL || strcmp(api_key, MYHTTPD_APIKEY) != 0) {
        LOG_WARN ("HTTP_UNAUTHORIZED");
        // evhttp_send_error(req, 401 , 0); // #define HTTP_UNAUTHORIZED   401
        // return;
    }

    /* Decode the URI */
    decoded = evhttp_uri_parse(evhttp_request_get_uri(req));
    if (!decoded) {
        evhttp_send_error(req, HTTP_BADREQUEST, 0);
        return;
    }

    /* Let's see what path the user asked for. */
    const char *path = evhttp_uri_get_path(decoded);
    if (!path) path = "/";

    /* We need to decode it, to see what path the user really wanted. */
    decoded_path = evhttp_uridecode(path, 0, NULL);
    if (decoded_path == NULL)
        goto err;
    // LOG_DEBUG ("decoded_path %s", decoded_path);

    struct api_route *r;
    HASH_FIND_STR(routes, decoded_path, r);
    if (r) {
        // LOG_TRACE ("route is %s, %p", r->path, r->cb);
        r->cb(req, arg);
        goto done;
    }

err:
    evhttp_send_error(req, HTTP_NOTFOUND, NULL);
done:
    if (decoded) evhttp_uri_free(decoded);
    if (decoded_path) free(decoded_path);
}

#define UNUSED(x) (void)(x)
static void status_handler(struct evhttp_request *req, void *arg)
{
    struct evbuffer *buf = NULL;
    struct app* myapp = NULL;
    cJSON* run_time = NULL;
    char* jsonstr = NULL;

    enum evhttp_cmd_type cmd = evhttp_request_get_command(req);
    LOG_DEBUG ("status_handler %d", cmd);

    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "application/json; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");

    // parse json
    char text[]="{\"status\":\"ok\",\"run_time\":\"\"}";
    cJSON * root = cJSON_Parse(text);
    if (!root) {
        LOG_ERROR ("cJSON_Parse error: %s", cJSON_GetErrorPtr());
        goto err;
    }
    myapp = (struct app*)arg;
    run_time = cJSON_GetObjectItemCaseSensitive(root, "run_time");
    if (run_time) cJSON_SetValuestring(run_time, asctime(localtime( &myapp->run_time )));
    jsonstr = cJSON_PrintUnformatted(root);

    buf = evbuffer_new();
    // evbuffer_add_printf(buf, "%s", jsonstr);
    evbuffer_add(buf, jsonstr, strlen(jsonstr));
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    free(jsonstr);
    cJSON_Delete(root);
    goto done;

err:
    evhttp_send_error(req, HTTP_INTERNAL, NULL);
done:
    if (buf) evbuffer_free(buf);
}

static void version_handler(struct evhttp_request *req, void *arg)
{
    UNUSED(arg);
    struct evbuffer *buf = NULL;

    enum evhttp_cmd_type cmd = evhttp_request_get_command(req);
    LOG_DEBUG ("status_handler %d", cmd);

    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "application/json; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");

    buf = evbuffer_new();
    evbuffer_add_printf(buf, "{\"version\": \"%s\", \"build_time\": \"%s %s\" }",
                        MY_VERSION, __DATE__, __TIME__);
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}

static void stats_handler(struct evhttp_request *req, void *arg)
{
    enum evhttp_cmd_type cmd = evhttp_request_get_command(req);
    LOG_DEBUG ("stats_handler %d", cmd);
    if (cmd != EVHTTP_REQ_GET) {
        evhttp_send_error(req, HTTP_NOTFOUND, NULL);
        return;
    }

    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "application/json; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");

    struct app* myapp = (struct app*)arg;
    // 这里取pcap统计后，会与后续的capture->count有差异,是因为时间差距，是正常的。
    // capture_stats(myapp->captr);

    char s[512] = {0};
//     snprintf(s, sizeof(s)-1, 
// "{ \"capture\": { \"pkts\": %zu, \"bytes\": %zu, "
// "\"pcap\": { \"ps_recv\": %d, \"ps_drop\": %d, \"ps_ifdrop\": %d"
// #ifdef _WIN32
// ", \"ps_capt\": %d, \"ps_sent\": %d, \"ps_netdrop\": %d"
// #endif
// " } },"
// "\"parser\": { \"pkts\": %llu, \"bytes\": %llu, "
// "\"word_match_count\": %llu, \"regex_match_count\": %llu }, "
// "\"inputer\": { \"count\": %zu },  "
// "\"worker\": { \"count\": %zu } }",
//         myapp->captr->pkts, myapp->captr->bytes,
//         myapp->captr->ps.ps_recv, myapp->captr->ps.ps_drop, myapp->captr->ps.ps_ifdrop,
// #ifdef _WIN32
//         myapp->captr->ps.ps_capt, myapp->captr->ps.ps_sent, myapp->captr->ps.ps_netdrop,
// #endif
//         atomic_load(&myapp->prsr->count), atomic_load(&myapp->prsr->bytes),
//         atomic_load(&myapp->prsr->word_match_count), atomic_load(&myapp->prsr->regex_match_count),
//         myapp->inptr->count, myapp->wrkr->count);
    struct evbuffer *buf;
    buf = evbuffer_new();
    evbuffer_add(buf, s, strlen(s));
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}

static void config_handler(struct evhttp_request *req, void *arg)
{
    enum evhttp_cmd_type cmd = evhttp_request_get_command(req);
    if (cmd != EVHTTP_REQ_GET) {
        evhttp_send_error(req, HTTP_NOTFOUND, NULL);
        return;
    }

    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "application/json; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");

    struct app* myapp = (struct app*)arg;
    UT_string* s = config2json(myapp->myconfig);
    if (!s) {
        evhttp_send_error(req, HTTP_INTERNAL, NULL);
        return;
    }

    struct evbuffer *buf;
    buf = evbuffer_new();
    evbuffer_add_printf(buf, "%s", utstring_body(s));
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
    utstring_free(s);
}

// dump internal data structure.
static void dump_handler(struct evhttp_request *req, void *arg)
{
    enum evhttp_cmd_type cmd = evhttp_request_get_command(req);
    if (cmd != EVHTTP_REQ_GET) {
        evhttp_send_error(req, HTTP_NOTFOUND, NULL);
        return;
    }

    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "application/json; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");

    struct app* myapp = (struct app*)arg;
    // sds s = parser_dump(myapp->prsr);
    // if (!s) {
    //     evhttp_send_error(req, HTTP_INTERNAL, NULL);
    //     return;
    // }

    struct evbuffer *buf;
    buf = evbuffer_new();
    // evbuffer_add_printf(buf, "{ \"parser\": %s", s);
    // sdsfree(s);

    // s = capture_dump(myapp->captr);
    // if (!s) {
    //     evhttp_send_error(req, HTTP_INTERNAL, NULL);
    //     evbuffer_free(buf);
    //     return;
    // }
    // evbuffer_add_printf(buf, ", \"capture\": %s }", s);
    // sdsfree(s);

    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}
