#include <pthread.h>
#include <sys/time.h>

#define cchan_mutex_t pthread_mutex_t
#define cchan_mutex_init(m) pthread_mutex_init(&m, NULL);
#define cchan_mutex_free(m) pthread_mutex_destroy(&m);
#define cchan_mutex_lock(m) pthread_mutex_lock(&m);
#define cchan_mutex_unlock(m) pthread_mutex_unlock(&m);

#define cchan_cond_t pthread_cond_t
#define cchan_cond_init(c) pthread_cond_init(&c, NULL);
#define cchan_cond_free(c) pthread_cond_destroy(&c);
#define cchan_cond_signal(c) pthread_cond_signal(&c);
#define cchan_cond_wait(c,m) pthread_cond_wait(&c, &m);

static inline void cchan_cond_waittime_1(pthread_cond_t *cond, pthread_mutex_t *mutex, int ms)
{
	struct timeval now;
	struct timespec timeout;

	gettimeofday(&now, NULL);
	timeout.tv_sec = now.tv_sec + ms / 1000;
	timeout.tv_nsec = (now.tv_usec + (ms % 1000) * 1000) * 1000;
	if(timeout.tv_nsec > 1000000000) {
		timeout.tv_nsec -= 1000000000;
		timeout.tv_sec ++;
	}

	pthread_cond_timedwait(cond, mutex, &timeout);
}

#define cchan_cond_waittime(c,m,ms) cchan_cond_waittime_1(&c,&m,ms);
