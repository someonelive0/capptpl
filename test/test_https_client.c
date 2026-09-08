// To build, run: 
// cc -g -DMG_TLS=MG_TLS_OPENSSL -I../lib/mongoose-7.21  test_https_client.c ../lib/mongoose-7.21/mongoose.c -lssl -lcrypto
#include "mongoose.h"


static volatile sig_atomic_t stopme = 0;
static void sig_handler(int sig);

// HTTP server event handler function
static void ev_handler(struct mg_connection *c, int ev, void *ev_data);

// static const char *s_url = "http://info.cern.ch/";
static const char *s_url = "https://192.168.30.39:9261/tag_rule";
static struct mg_str s_ca_pem;              // CA PEM file
static const char *s_post_data = NULL;      // POST data
static const uint64_t s_timeout_ms = 1500;  // Connect timeout in milliseconds


int main(void)
{
    signal(SIGINT, sig_handler);
    printf("Connecting to %s", s_url);

    struct mg_mgr mgr;                                             // Declare event manager
    mg_log_set(MG_LL_DEBUG);                                       // Default set loglevel to DEBUG
    mg_mgr_init(&mgr);                                             // Initialise event manager
    struct mg_connection *conn = mg_http_connect(&mgr, s_url, ev_handler, (void*)&stopme);
    if (conn == NULL) {
        printf("Failed to connect to %s", s_url);
        exit(1);
    }
    while(!stopme) {                              // Run an infinite event loop
        mg_mgr_poll(&mgr, 200);            // poll 1 seconds then timeout
    }
    mg_mgr_free(&mgr);                        // Free resources
}


// HTTP server event handler function
static void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_OPEN) {
        // Connection created. Store connect expiration time in c->data
        *(uint64_t *) c->data = mg_millis() + s_timeout_ms;

    } else if (ev == MG_EV_POLL) {
        if (mg_millis() > *(uint64_t *) c->data &&
            (c->is_connecting || c->is_resolving)) {
          mg_error(c, "Connect timeout");
        }

    } else if (ev == MG_EV_CONNECT) {
        // Connected to server. Extract host name from URL
        struct mg_str host = mg_url_host(s_url);
    
        if (c->is_tls) {
          struct mg_tls_opts opts = {.ca = s_ca_pem, .name = mg_url_host(s_url)};
          mg_tls_init(c, &opts);
        }
    
        // Send request
        int content_length = s_post_data ? strlen(s_post_data) : 0;
        mg_printf(c,
                  "%s %s HTTP/1.0\r\n"
                  "Host: %.*s\r\n"
                  "User-Agent: Mongoose " MG_VERSION "\r\n"
                  "Content-Type: octet-stream\r\n"
                  "Content-Length: %d\r\n"
                  "\r\n",
                  s_post_data ? "POST" : "GET", mg_url_uri(s_url), (int) host.len,
                  host.buf, content_length);
        mg_send(c, s_post_data, content_length);

    } else if (ev == MG_EV_HTTP_MSG) {
        // Response is received. Print it
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        printf("----> status %d\n", mg_http_status(hm));
        printf("----MSG> %.*s\n", (int) hm->message.len, hm->message.buf);
        printf("----BODY> %.*s\n", (int) hm->body.len, hm->body.buf);
        c->is_draining = 1;           // Tell mongoose to close this connection
        *(int *) c->fn_data = 1;      // Tell event loop to stop

    } else if (ev == MG_EV_ERROR) {
        *(int *) c->fn_data = 1;      // Error, tell event loop to stop
    }
}

static void sig_handler(int sig) {
    printf("Caught signal %d: %s\n", sig, strsignal(sig));
    stopme = 1; // 设置退出标志
}
