#include <stdio.h>
#include <stdlib.h>

#include "cchan_sdl.h"

cchan_t *chan1 = NULL;    /* producers -> consumers */
cchan_t *chan2 = NULL;    /* consumers -> main */
cchan_t *chandead = NULL; /* dead producers / dead consumers signaling */

int producer(void *data)
{
	int amt = ((long) data);
	int i, v;
	int slp;

	printf("start producer of %d\n", amt);

	for(i=0; i<amt; i++) {
#ifndef NOWAIT
		slp = random() % 1000;
		usleep(slp);
#endif

		v = random() % 1000;

		cchan_send(chan1, &v);
	}

	printf("exit producer of %d\n", amt);

#ifndef NOWAIT
	usleep(random() % 10000000);
#endif

	v = 1;
	cchan_send(chandead, &v);

	return 0;
}

int consumer(void *data)
{
	int amt = ((long) data);
	int i, v;

	printf("start consumer of %d\n", amt);

	for(i=0; i<amt; i++) {
		cchan_wait(chan1, &v);
		cchan_send(chan2, &v);
	}

#ifndef NOWAIT
	usleep(random() % 10000000);
#endif

	v = 2;
	cchan_send(chandead, &v);

	return 0;
}

int main(int argc, char *argv[])
{
	int i,v;
	long arg;

	SDL_Init(0);

	chan1 = cchan_new(sizeof(int));
	chan2 = cchan_new(sizeof(int));
	chandead = cchan_new(sizeof(int));

	/* create 10 * 1000 consumer */

	arg = 1000;
	for(i=0; i<10; i++) {
		SDL_CreateThread(consumer, (void *) arg);
	}

	/* create 10000 producer */

	arg = 50;
	SDL_CreateThread(producer, (void *) arg);

	arg = 100;
	SDL_CreateThread(producer, (void *) arg);

	arg = 500;
	SDL_CreateThread(producer, (void *) arg);

	arg = 1000;
	SDL_CreateThread(producer, (void *) arg);

	arg = 2000;
	SDL_CreateThread(producer, (void *) arg);

	arg = 3000;
	SDL_CreateThread(producer, (void *) arg);

	arg = 3350;
	SDL_CreateThread(producer, (void *) arg);

	/* wait for output */

	for(i=0; i<10000; i++) {
		cchan_wait(chan2, &v);
		printf("chan2[%d] = %d\n", i, v);
	}

	/* wait for death reports */

	for(i=17; i>0;) {
		if(cchan_waittime(chandead, &v, 100)) {
			printf("\ndead: %d\n", v);
			i --;
		} else {
			printf(".");
			fflush(stdout);
		}
	}

	cchan_free(chan1);
	cchan_free(chan2);
	cchan_free(chandead);

	SDL_Quit();

	return 0;
}
