#ifndef DOREDIS_H
#define DOREDIS_H

#include <event2/event.h>


int redis_connect(char* addr, int port, struct event_base* evbase);
int redis_close();

#endif // DOREDIS_H
