#ifndef MAGT_H
#define MAGT_H

#include <event2/http.h>
#include <event2/http_struct.h>

#include "apptpl2_init.h"

#define MYHTTPD_SIGNATURE   "myhttpd v 0.0.1"
#define MYHTTPD_APIKEY      "13bd70626cba23d23ad40ee53548c346b4c7da1d"


extern struct event_base *magt_evbase;

int magt_init(const struct config* myconfig);
int magt_close();
void* magt_loop(const struct config* myconfig);

int api_route_init();
void api_route_free();
void api_handler(struct evhttp_request *req, void *arg);

// management interface thread
// param *data is int port
// void* magt(void *arg);

#endif // MAGT_H
