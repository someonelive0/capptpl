#include <stdio.h>

#include "cchan_pthread.h"

void pstate(cchan_t *chan)
{
	int i, v;

	printf("----------------------- chan %p\n", (void *) chan);
	printf("alloc=%d next=%d first=%d used=%d\n", chan->alloc, chan->next, chan->first, chan->used);

	printf("[");
	for(i=0; i<chan->alloc; i++) {
		v = * (int *) (chan->data + i * chan->size);
		if(v >= 'a' && v <= 'z')
			printf("%c", v);
		else
			printf(".");
	}
	printf("]\n");

	printf("\n");
}

void precv(cchan_t *chan)
{
	int out;
	int res = cchan_recv(chan, &out);

	if(!res) {
		printf("recv fail\n");
	} else {
		printf("recv = %c\n", out);
	}
}

int main(int argc, char *argv[])
{
	cchan_t *c1 = cchan_new(sizeof(int));
	int v;

	pstate(c1);

	v = 'a';
	cchan_send(c1, &v);

	pstate(c1);

	v = 'b';
	cchan_send(c1, &v);
	v = 'c';
	cchan_send(c1, &v);

	pstate(c1);

	precv(c1);
	precv(c1);

	pstate(c1);

	v = 'd';
	cchan_send(c1, &v);
	v = 'e';
	cchan_send(c1, &v);
	v = 'f';
	cchan_send(c1, &v);

	pstate(c1);

	v = 'g';
	cchan_send(c1, &v);

	pstate(c1);

	precv(c1);
	precv(c1);
	precv(c1);
	precv(c1);
	precv(c1);
	precv(c1);

	cchan_free(c1);

	return 0;
}
