#ifndef MAGT_H
#define MAGT_H

#define MYHTTPD_SIGNATURE   "myhttpd v 0.0.1"


int magt_init(struct config* myconfig);
int magt_close();
void* magt_loop(struct config* myconfig);

// management interface thread
// param *data is int port
void* magt(void *arg);

#endif // MAGT_H
