#include "doredis.h"

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

#include "logger.h"


static char *redis_host;
static int redis_port;
static char *redis_passwd;
static char *redis_list;
static char redis_cmd[128];
static struct redisAsyncContext *redis_ctx;
static struct event_base* redis_evbase;
static int redis_status;

static void on_redis_connect(const struct redisAsyncContext *ctx, int status);
static void on_redis_close(const struct redisAsyncContext *ctx, int status);
static void on_redis_auth(struct redisAsyncContext *ctx, void *reply, void* arg);
static void on_redis_pop(struct redisAsyncContext *ctx, void *reply, void* arg);
static void do_redis_pop(evutil_socket_t fd, short arg, void * ctx);
static void redis_reconnect(evutil_socket_t fd, short arg, void * ctx);


int redis_connect(char* host, int port, char* passwd, char* list, struct event_base* evbase)
{
    if (redis_ctx) {
        LOG_ERROR ("redis had connected");
        return -1;
    }
    redis_evbase = evbase;

    struct redisAsyncContext *ctx = redisAsyncConnect(host, port);
    if (!ctx || ctx->err) {
        LOG_ERROR ("redisAsyncConnect: %s", ctx ? ctx->errstr : "error");
        return -1;
    }

    if (0 != redisLibeventAttach(ctx, evbase)) {
        LOG_ERROR ("redisLibeventAttach failed");
        redisAsyncDisconnect(ctx);
        redisAsyncFree(ctx);
        return -1;
    }
    redis_ctx = ctx;
    redis_host = host;
    redis_port = port;
    redis_passwd = passwd;
    redis_list = list;
    int expired = 10; // expired 10 seconds
    snprintf(redis_cmd, sizeof(redis_cmd)-1, "BRPOP %s %d", redis_list, expired);
    redis_cmd[strlen(redis_cmd)] = '\0';

    (void)redisAsyncSetConnectCallback(ctx, on_redis_connect);
    (void)redisAsyncSetDisconnectCallback(ctx, on_redis_close);

    return 0;
}

int redis_close()
{
    if (redis_ctx) { // } && redis_status == REDIS_OK) {
        LOG_DEBUG ("Redis disconnect...");
        redisAsyncDisconnect(redis_ctx);
        redisAsyncFree(redis_ctx);
        redis_ctx = NULL;
        return 0;
    }

    return -1;
}

static void on_redis_connect(const struct redisAsyncContext *ctx, int status)
{
    redis_status = status;
    if (status == REDIS_ERR) {
        LOG_ERROR ("Redis connect error %s, should reconnect after 5 seconds", ctx->errstr);
        // redisAsyncDisconnect(redis_ctx);
        redisAsyncFree(redis_ctx);
        redis_ctx = NULL;

        // after 5 second to reconnect redis
        struct timeval timeout = {5, 0};
        event_base_once(redis_evbase, -1, EV_TIMEOUT, redis_reconnect, (void*)ctx, &timeout);
        // redis_connect(redis_host, redis_port, redis_passwd, redis_evbase);
        return;
    }

    if (status == REDIS_OK) {
        LOG_INFO ("Redis to %s:%d connected", redis_host, redis_port);
        if (strlen(redis_passwd) > 0) { // if need auth
            char tmp[64] = {0};
            snprintf(tmp, sizeof(tmp)-1, "AUTH %s", redis_passwd);
            if (REDIS_OK != redisAsyncCommand((struct redisAsyncContext *)ctx,
                                              on_redis_auth, NULL, tmp)) {
                LOG_ERROR ("Redis async auth error %s", ctx->errstr);
            }
        } else {
            struct timeval timeout = {0, 500};
            event_base_once(redis_evbase, -1, EV_TIMEOUT, do_redis_pop, (void*)ctx, &timeout);
            // redisAsyncCommand((struct redisAsyncContext *)ctx, on_redis_pop, NULL, "BRPOP queue1 queue2 queue3 10");
        }
    }
}

#define UNUSED(x) (void)(x)
static void on_redis_close(const struct redisAsyncContext *ctx, int status)
{
    UNUSED(ctx);
    UNUSED(status);
    LOG_INFO ("Redis disconnected, status: %d", status);
}

static void on_redis_auth(struct redisAsyncContext *ctx, void *reply, void *arg)
{
    UNUSED(arg);
    struct redisReply *redis_reply = reply;
    if (!redis_reply || redis_reply->type == REDIS_REPLY_ERROR) {
        LOG_ERROR ("redis auth error: %s", redis_reply ? redis_reply->str : "error");
        redis_status = REDIS_ERR;
        return;
    } else if (redis_reply->type != REDIS_REPLY_STATUS) {
        LOG_ERROR ("redis auth unknown type: %d", redis_reply->type);
        redis_status = REDIS_ERR;
        return;
    }
    LOG_INFO ("redis AUTH success");

    if (REDIS_OK != redisAsyncCommand(ctx, on_redis_pop, NULL, redis_cmd)) {
        LOG_ERROR ("Redis async '%s' error %s", redis_cmd, ctx->errstr);
    }
}

static void do_redis_pop(evutil_socket_t fd, short arg, void* ctx)
{
    UNUSED(fd);
    UNUSED(arg);
    struct redisAsyncContext *tmpctx = (struct redisAsyncContext *)ctx;
    LOG_INFO ("redis begin BRPOP = '%s' ...", redis_cmd);
    if (REDIS_OK != redisAsyncCommand(tmpctx, on_redis_pop, NULL, redis_cmd)) {
        LOG_ERROR ("Redis async '%s' error %s", redis_cmd, tmpctx->errstr);
    }
}

static void on_redis_pop(struct redisAsyncContext *ctx, void *reply, void *arg)
{
    UNUSED(arg);
    struct redisReply *redis_reply = reply;
    if (!redis_reply) {
        LOG_DEBUG ("redis pop get NULL reply");
        return;
    } else if (redis_reply->type == REDIS_REPLY_ERROR) {
        LOG_ERROR ("redis pop failed: %s", redis_reply ? redis_reply->str : "reply NULL");
        redis_status = REDIS_ERR;
        return;
    }

    if (redis_reply->type == REDIS_REPLY_NIL) {
        LOG_TRACE ("BRPOP: empty and expired, continue ...");
    } else if (redis_reply->type == REDIS_REPLY_ARRAY) {
        LOG_TRACE ("BRPOP: array with length %lld", redis_reply->elements);
        for (size_t i=0; i<redis_reply->elements; i++) {
            struct redisReply *rs = redis_reply->element[i];
            LOG_TRACE ("\t%d: %s", i, rs->str);
        }
    }

    if (REDIS_OK != redisAsyncCommand(ctx, on_redis_pop, NULL, redis_cmd)) {
        LOG_ERROR ("Redis async '%s' error %s", redis_cmd, ctx->errstr);
    }
}

static void redis_reconnect(evutil_socket_t fd, short arg, void * ctx)
{
    UNUSED(fd);
    UNUSED(arg);
    UNUSED(ctx);
    redis_connect(redis_host, redis_port, redis_passwd, redis_list, redis_evbase);
}
