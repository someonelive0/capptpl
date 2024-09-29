
/*
 * management of http services.
 */
#include "magt.h"

#include <stdlib.h>
#include <signal.h>
#include <event.h>
#include <event2/http_compat.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "logger.h"


struct event_base *magt_evbase;
static struct evhttp *magt_httpd;
static SSL_CTX *magt_sslctx;
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

static SSL_CTX* ssl_init(const struct config* myconfig);
static struct bufferevent* bufev_ssl_cb(struct event_base *base, void *arg);


int magt_init(const struct config* myconfig)
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
    struct event *evtimer = event_new(base, -1, EV_TIMEOUT|EV_PERSIST, timer_cb, base);
    struct timeval timeout = {2, 0};
    evtimer_add(evtimer, &timeout); //  event_add(evtimer, 0);

    // add httpd event
    struct evhttp* httpd = evhttp_new(base);
    if (!httpd) {
        LOG_ERROR ("create evhttp failed!");
        return -1;
    }

    SSL_CTX* sslctx = NULL;
    if (myconfig->enable_ssl) {
        sslctx = ssl_init(myconfig);
        if (!sslctx) {
            LOG_ERROR ("ssl_init failed!");
            return -1;
        }
        /*
            使我们创建好的evhttp句柄 支持 SSL加密
            实际上，加密的动作和解密的动作都已经帮
            我们自动完成，我们拿到的数据就已经解密之后的
        */
        evhttp_set_bevcb (httpd, bufev_ssl_cb, sslctx);
    }

    if (evhttp_bind_socket(httpd, "0.0.0.0", myconfig->http_port) != 0) {
        LOG_ERROR ("bind socket failed! port:%d", myconfig->http_port);
        return -1;
    }

    if (-1 == api_route_init())
        return -1;
    evhttp_set_gencb(httpd, api_handler, (struct config*)myconfig);

    magt_evbase = base;
    magt_httpd = httpd;
    magt_sigint = evsigint;
    magt_sigterm = evsigterm;
    magt_timer = evtimer;
    magt_sslctx = sslctx;
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
    SSL_CTX_free(magt_sslctx);
    api_route_free();

    return 0;
}

void* magt_loop(const struct config* myconfig)
{
    if (myconfig->enable_ssl)
        LOG_INFO ("https listen port %d, access https://localhost:%d"
                  "\tstart event loop", myconfig->http_port, myconfig->http_port);
    else
        LOG_INFO ("http listen port %d, access http://localhost:%d"
                  "\tstart event loop", myconfig->http_port, myconfig->http_port);
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
    UNUSED(fd);
    UNUSED(event);
    UNUSED(arg);
    // LOG_TRACE ("%s: got timeout with unix time: %lld\n", __func__, time(NULL));
    logger_flush();
}


/* OpenSSL has a habit of using uninitialized memory.  (They turn up their
 * nose at tools like valgrind.)  To avoid spurious valgrind errors (as well
 * as to allay any concerns that the uninitialized memory is actually
 * affecting behavior), let's install a custom malloc function which is
 * actually calloc.
 */
// static void *my_zeroing_malloc (size_t howmuch)
// {
//     return calloc (1, howmuch);
// }

SSL_CTX* ssl_init(const struct config* myconfig)
{
    SSL_CTX *ctx = NULL;

    // CRYPTO_set_mem_functions (my_zeroing_malloc, realloc, free);
    SSL_library_init ();
    SSL_load_error_strings ();
    OpenSSL_add_all_algorithms ();
    /* We MUST have entropy, or else there's no point to crypto. */
    if (!RAND_poll()) {
        LOG_ERROR ("RAND_poll failed %d: %s",
                    ERR_get_error(), ERR_error_string(ERR_get_error(), NULL));
        goto err;
    }

    LOG_INFO ("Using OpenSSL version \"%s\", libevent version \"%s\"",
              SSLeay_version (SSLEAY_VERSION), event_get_version ());

    /* 创建SSL上下文环境 ，可以理解为 SSL句柄 */
    ctx = SSL_CTX_new (SSLv23_server_method ());
    if (NULL == ctx) {
        LOG_ERROR ("SSL_CTX_new failed %d: %s",
                    ERR_get_error(), ERR_error_string(ERR_get_error(), NULL));
        goto err;
    }

    SSL_CTX_set_options (ctx,
                         SSL_OP_SINGLE_DH_USE |
                         SSL_OP_SINGLE_ECDH_USE |
                         SSL_OP_NO_SSLv2);

    /* Cheesily pick an elliptic curve to use with elliptic curve ciphersuites.
        * We just hardcode a single curve which is reasonably decent.
        * See http://www.mail-archive.com/openssl-dev@openssl.org/msg30957.html */
    // EC_KEY *ecdh = EC_KEY_new_by_curve_name (NID_X9_62_prime256v1);
    // if (! ecdh) {
    //     LOG_ERROR ("EC_KEY_new_by_curve_name failed %d: %s", ERR_get_error(), ERR_error_string(ERR_get_error(), NULL));
    //     goto err;
    // }
    // if (1 != SSL_CTX_set_tmp_ecdh (ctx, ecdh)) {
    //     LOG_ERROR ("SSL_CTX_set_tmp_ecdh failed %d: %s", ERR_get_error(), ERR_error_string(ERR_get_error(), NULL));
    //     goto err;
    // }

    /* 选择服务器证书 和 服务器私钥. */
    // const char *certificate_chain = "server.crt";
    // const char *private_key = "server.key";

    /* 设置服务器证书 和 服务器私钥 到OPENSSL ctx上下文句柄中
      首先生成RSA私钥(使用aes256加密)：openssl genrsa -aes256 -out server1.key 2048，
      再根据该私钥生成一个证书：openssl req -new -x509 -days 3650 -key server1.key -out server.crt
      私钥转非加密 openssl rsa -in server1.key -passin pass:123456 -out server.key
     */
    LOG_INFO ("Loading SSL certificate chain and key from files '%s', "
              "'%s'", myconfig->crt_file, myconfig->key_file);

    if (1 != SSL_CTX_use_certificate_chain_file (ctx, myconfig->crt_file)) {
        LOG_ERROR ("SSL_CTX_use_certificate_chain_file failed %d: %s",
                    ERR_get_error(), ERR_error_string(ERR_get_error(), NULL));
        goto err;
    }

    if (1 != SSL_CTX_use_PrivateKey_file (ctx, myconfig->key_file, SSL_FILETYPE_PEM)) {
        LOG_ERROR ("SSL_CTX_use_PrivateKey_file failed %d: %s",
                    ERR_get_error(), ERR_error_string(ERR_get_error(), NULL));
        goto err;
    }

    if (1 != SSL_CTX_check_private_key (ctx)) {
        LOG_ERROR ("SSL_CTX_check_private_key failed %d: %s",
                    ERR_get_error(), ERR_error_string(ERR_get_error(), NULL));
        goto err;
    }

    return ctx;

err:
    if (ctx) SSL_CTX_free(ctx);
    return NULL;
}

/**
 * This callback is responsible for creating a new SSL connection
 * and wrapping it in an OpenSSL bufferevent.  This is the way
 * we implement an https server instead of a plain old http server.
 */
static struct bufferevent* bufev_ssl_cb (struct event_base *base, void *arg)
{
    struct bufferevent* r;
    SSL_CTX *ctx = (SSL_CTX *) arg;

    r = bufferevent_openssl_socket_new (base,
                                        -1,
                                        SSL_new (ctx),
                                        BUFFEREVENT_SSL_ACCEPTING,
                                        BEV_OPT_CLOSE_ON_FREE);
    return r;
}
