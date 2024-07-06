#include "SDL.h"
#include "SDL_thread.h"

#define cchan_mutex_t SDL_mutex *
#define cchan_mutex_init(m) m = SDL_CreateMutex();
#define cchan_mutex_free(m) SDL_DestroyMutex(m);
#define cchan_mutex_lock(m) SDL_mutexP(m);
#define cchan_mutex_unlock(m) SDL_mutexV(m);

#define cchan_cond_t SDL_cond *
#define cchan_cond_init(c) c = SDL_CreateCond();
#define cchan_cond_free(c) SDL_DestroyCond(c);
#define cchan_cond_signal(c) SDL_CondSignal(c);
#define cchan_cond_wait(c,m) SDL_CondWait(c, m);

#define cchan_cond_waittime(c,m,ms) SDL_CondWaitTimeout(c, m, ms);
