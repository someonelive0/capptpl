#ifndef MAGT_H
#define MAGT_H

#define MYHTTPD_SIGNATURE   "myhttpd v 0.0.1"


// management interface thread
// param *data is int port
// int magt_init();
// int magt_close();
void* magt(void *arg);

#endif // MAGT_H
