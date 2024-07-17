#ifndef MAGT_H
#define MAGT_H

#define MYHTTPD_SIGNATURE   "myhttpd v 0.0.1"

extern struct event_base *magt_evbase;

int magt_init(struct config* myconfig);
int magt_close();
void* magt_loop(struct config* myconfig);

int api_route_init();
void api_route_free();
void api_handler(struct evhttp_request *req, void *arg);

// management interface thread
// param *data is int port
void* magt(void *arg);

#endif // MAGT_H
