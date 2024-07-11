#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>
#include <event2/event.h>

#include "logger.h"


struct redisAsyncContext *redis_ctx;
struct event_base* tmp_evbase;
int redis_status;
static void on_redis_connect(const struct redisAsyncContext *redis_ctx, int status);
static void on_redis_close(const struct redisAsyncContext *redis_ctx, int status);
static void on_redis_auth(struct redisAsyncContext *redis_ctx, void *reply, void* arg);
static void on_redis_pop(struct redisAsyncContext *redis_ctx, void *reply, void* arg);
static void do_redis_pop(evutil_socket_t, short, void * ctx);


int redis_connect(char* addr, int port, struct event_base* evbase) {
    if (redis_ctx) {
        LOG_ERROR ("redis had connected");
        return -1;
    }
    tmp_evbase = evbase;

    struct redisAsyncContext *ctx = redisAsyncConnect(addr, port);
    if (!ctx || ctx->err) {
        LOG_ERROR ("redisAsyncConnect: %s", ctx ? ctx->errstr : "error");
        return -1;
    }

    if (0 != redisLibeventAttach(ctx, evbase)) {
        LOG_ERROR ("redisLibeventAttach failed");
        redisAsyncDisconnect(ctx);
        return -1;
    }
    redis_ctx = ctx;

    (void)redisAsyncSetConnectCallback(redis_ctx, on_redis_connect);
    (void)redisAsyncSetDisconnectCallback(redis_ctx, on_redis_close);

    return 0;
}

int redis_close() {
    if (redis_ctx) { // } && redis_status == REDIS_OK) {
        LOG_DEBUG ("Redis disconnect...");
        redisAsyncDisconnect(redis_ctx); // __redisAsyncFree() called
        redis_ctx = NULL;
        return 0;
    }

    return -1;
}


static void on_redis_connect(const struct redisAsyncContext *redis_ctx, int status) {
    redis_status = status;
    if (status == REDIS_ERR) {
        LOG_ERROR ("Redis connect error %s, reconnecting...", redis_ctx->errstr);
        return;
    }

    if (status == REDIS_OK) {
        LOG_INFO ("Redis connected");
        if (0) { // if need auth
            redisAsyncCommand((struct redisAsyncContext *)redis_ctx, on_redis_auth, NULL, "AUTH BDsec2022,,");
        } else {
            struct timeval timeout = {0, 500};
            event_base_once(tmp_evbase, -1, EV_TIMEOUT, do_redis_pop, (void*)redis_ctx, &timeout);
            // redisAsyncCommand((struct redisAsyncContext *)redis_ctx, on_redis_pop, NULL, "BRPOP queue1 queue2 queue3 10");
        }
    }
}

static void on_redis_close(const struct redisAsyncContext *, int status) {
    LOG_INFO ("Redis disconnected: %d", status);
}

static void on_redis_auth(struct redisAsyncContext *redis_ctx, void *reply, void* ) {
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

    redisAsyncCommand(redis_ctx, on_redis_pop, NULL, "BRPOP queue1 queue2 queue3 10");
}

static void do_redis_pop(evutil_socket_t, short, void* ctx) {
    LOG_INFO ("redis begin BRPOP ...");
    redisAsyncCommand((struct redisAsyncContext *)ctx, on_redis_pop, NULL, "BRPOP queue1 queue2 queue3 10");
}

static void on_redis_pop(struct redisAsyncContext *redis_ctx, void *reply, void* ) {
    struct redisReply *redis_reply = reply;
    if (!redis_reply || redis_reply->type == REDIS_REPLY_ERROR) {
        LOG_ERROR ("redis pop failed: %s", redis_reply ? redis_reply->str : "error");
        redis_status = REDIS_ERR;
        return;
    }

    if (redis_reply->type == REDIS_REPLY_NIL) {
        LOG_DEBUG ("BRPOP: empty...");
    } else if (redis_reply->type == REDIS_REPLY_ARRAY) {
        LOG_DEBUG ("BRPOP: array with length %lld", redis_reply->elements);
        for (size_t i=0; i<redis_reply->elements; i++) {
            struct redisReply *rs = redis_reply->element[i];
            LOG_INFO ("BRPOP: %s", rs->str);
        }
    }

    redisAsyncCommand(redis_ctx, on_redis_pop, NULL, "BRPOP queue1 queue2 queue3 10");
}
