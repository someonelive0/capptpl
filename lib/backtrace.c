#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <stddef.h>   // NULL
#include <setjmp.h>   // For potential recovery scenarios
#ifdef __linux__
#include <execinfo.h> // callstack()
#include <dlfcn.h>    // callstack()
#endif

#include "logger.h"


#ifdef __linux__
/*
 * for print stack backtrace inside max_frames
 * first print to STDOUT, then to fp.
 * 
 * you need the -ggdb option to gcc.
 * also you need the -rdynamic option to gcc, 
 * which makes it pass a flag to the linker which ensures that all symbols are placed in the table
 */
char* log_backtrace(const int max_frames, int fd) {
    void *callstack[max_frames];
    int n_frames = backtrace(callstack, max_frames);

    // first output to STDOUT
    printf("backtrace() returned %d addresses\n", n_frames);
    backtrace_symbols_fd(callstack, n_frames, 1); // STDOUT

    // thenn output to fd if fd > 0(STDIN)
    FILE* fp = NULL;
    if (fd > 0) fp = fdopen(fd, "w");
    if (fp != NULL) {
        fflush(fp);
        fprintf(fp, "backtrace() returned %d addresses\n", n_frames);

        char **symbols = backtrace_symbols(callstack, n_frames);
        for (int i = 0; i < n_frames; i++) {
            Dl_info info;
            if (dladdr(callstack[i], &info) && info.dli_sname != NULL) {
                fprintf(fp, "%-2d: %p\t%s\n", i, callstack[i], info.dli_sname);
            } else {
                fprintf(fp, "%-2d: %p\t%s\n", i, callstack[i], symbols[i]);
            }
        }
        free(symbols);
        if (n_frames >= max_frames)
            fprintf(fp, "[truncated]\n");
        fflush(fp);
    }

    return NULL;
}


/*
 * for register catch SIGSEGV
 */
// static jmp_buf catch_segfault; // For potential recovery

/*
 * param sig should be SIGSEGV
 */
#define UNUSED(x) (void)(x)
static void segfault_handler(int sig, siginfo_t *info, void *secret) {
    UNUSED(secret);
    LOG_FATAL("Crashed by signal: %d, si_code: %d", sig, info->si_code);
    if (sig == SIGSEGV || sig == SIGBUS) {
        LOG_FATAL("Caught segmentation fault (SIGSEGV | SIGBUS) %d !!!", sig);
        if (info->si_addr != NULL)
            LOG_WARN("Accessing address: %p", (void*)info->si_addr);
    }
    if (info->si_code == SI_USER && info->si_pid != -1) {
        LOG_WARN("Killed by PID: %ld, UID: %d", (long) info->si_pid, info->si_uid);
    }
    logger_flush();

    // now print backtrce
    int fd = logger_filefd();
    if (fd == -1) {
        LOG_ERROR("get logger file fd failed: %d, %s", errno, strerror(errno));
    }
    log_backtrace(100, fd);

    // Perform cleanup, log information, etc.
    // It's generally not safe to continue execution after a segfault.
    // However, for specific scenarios, you might attempt recovery using longjmp.
    // longjmp(catch_segfault, 1); // Jump back to a safe point
}

int set_sigsegv_handler() {
    // this is too simple, always printf code after wrong line of code
    // if (signal(SIGSEGV, segfault_handler) == SIG_ERR) {
    //     LOG_ERROR("Failed to register signal handler for SIGSEGV");
    //     return 1;
    // }

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
    act.sa_sigaction = segfault_handler;

    sigaction(SIGSEGV, &act, NULL);
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGFPE, &act, NULL);
    sigaction(SIGILL, &act, NULL);
    sigaction(SIGABRT, &act, NULL);

    // if (setjmp(catch_segfault) == 0) {
    //     LOG_DEBUG("set_sigsegv_handler ok.");
    //     // Code that might cause a segfault for test
    //     // int *ptr = NULL;
    //     // *ptr = 10; // This will cause a segfault
    // } else {
    //     LOG_INFO("Recovered from segmentation fault (or attempted recovery).");
    //     // Handle the aftermath of the potential segfault
    // }

    return 0;
}

#else

int set_sigsegv_handler() { return 0; }

#endif // __linux__

