#include <stdlib.h>
#include <string.h>

/* create a new channel */
cchan_t * cchan_new(int valuesize)
{
	cchan_t * c = malloc(sizeof *c);
	if(!c) abort();

	c->size = valuesize;
	c->alloc = 1;
	c->next = 0;
	c->first = 0;
	c->used = 0;
	c->data = malloc(c->alloc * valuesize);
	if(!c->data) abort();

	cchan_mutex_init(c->mutex);
	cchan_cond_init(c->cond);

	return c;
}

/* destroy channel (not thread-safe..) */
void cchan_free(cchan_t *chan)
{
	cchan_mutex_free(chan->mutex);
	cchan_cond_free(chan->cond);

	free(chan->data);
	free(chan);
}

/* put value in channel (never blocks) */
void cchan_send(cchan_t *chan, void *value)
{
	int i;

	cchan_mutex_lock(chan->mutex);

	if(chan->used == chan->alloc) {
		/* need to expand channel buffer */

		i = chan->alloc; /* save old buffer size */

		chan->alloc *= 2;

		chan->data = realloc(chan->data, chan->alloc * chan->size);
		if(!chan->data) abort();

		/* move part of buffer from beginning until the first contained entry
		 * after the previous end of the buffer
		 * [34512] ->
		 * [   12345   ]
		 */
		memcpy(chan->data + i * chan->size, chan->data, chan->first * chan->size);
		chan->next = i + chan->first;
	}

	memcpy(chan->data + chan->next * chan->size, value, chan->size);
	chan->next ++;
	if(chan->next == chan->alloc) chan->next = 0;
	chan->used ++;

	cchan_cond_signal(chan->cond);

	cchan_mutex_unlock(chan->mutex);
}

/* get value from channel (return nonzero on success, doesn't block) */
int cchan_recv(cchan_t *chan, void *output)
{
	cchan_mutex_lock(chan->mutex);
	if(!chan->used) {
		cchan_mutex_unlock(chan->mutex);
		return 0;
	}

	memcpy(output, chan->data + chan->first * chan->size, chan->size);

	chan->first ++;
	if(chan->first == chan->alloc) chan->first = 0;

	chan->used --;

	cchan_mutex_unlock(chan->mutex);

	return 1;
}

/* get value from channel (blocks until a value is available) */
void cchan_wait(cchan_t *chan, void *output)
{
	cchan_mutex_lock(chan->mutex);

	for(;;) {
		if(chan->used) {
			memcpy(output, chan->data + chan->first * chan->size, chan->size);
			chan->first ++;
			if(chan->first == chan->alloc) chan->first = 0;
			chan->used --;
			cchan_mutex_unlock(chan->mutex);
			return;
		}

		cchan_cond_wait(chan->cond, chan->mutex);
	}
}

/* get value from channel
 * blocks until a value is available, or ms milliseconds have elapsed
 * returns nonzero on success, zero on timeout
 */
int cchan_waittime(cchan_t *chan, void *output, int ms)
{
	cchan_mutex_lock(chan->mutex);

	if(chan->used) {
		memcpy(output, chan->data + chan->first * chan->size, chan->size);
		chan->first ++;
		if(chan->first == chan->alloc) chan->first = 0;
		chan->used --;
		cchan_mutex_unlock(chan->mutex);
		return 1;
	}

	cchan_cond_waittime(chan->cond, chan->mutex, ms);

	if(chan->used) {
		memcpy(output, chan->data + chan->first * chan->size, chan->size);
		chan->first ++;
		if(chan->first == chan->alloc) chan->first = 0;
		chan->used --;
		cchan_mutex_unlock(chan->mutex);
		return 1;
	} else {
		cchan_mutex_unlock(chan->mutex);
		return 0;
	}
}
