typedef struct {
	int alloc;   /* allocated entries in *data */
	int next;    /* index of next written entry */
	int first;   /* index of first contained entry */
	int used;    /* number of used entries */
	int size;    /* size of one value */
	char *data;  /* queue data */

	cchan_mutex_t mutex; /* for locking the channel */
	cchan_cond_t cond;   /* for signalling availability of data */
} cchan_t;

/* create a new channel */
cchan_t * cchan_new(int valuesize);

/* destroy channel (not thread-safe..) */
void cchan_free(cchan_t *chan);

/* put value in channel (never blocks) */
void cchan_send(cchan_t *chan, void *value);

/* get value from channel (return nonzero on success, doesn't block) */
int cchan_recv(cchan_t *chan, void *output);

/* get value from channel (blocks until a value is available) */
void cchan_wait(cchan_t *chan, void *output);

/* get value from channel
 * blocks until a value is available, or ms milliseconds have elapsed
 * returns nonzero on success, zero on timeout
 */
int cchan_waittime(cchan_t *chan, void *output, int ms);
