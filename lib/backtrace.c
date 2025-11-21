#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <stddef.h>   // NULL
#include <setjmp.h>   // For potential recovery scenarios
#include <execinfo.h> // callstack()
#include <dlfcn.h>    // callstack()

#include "logger.h"


/*
 * for print stack backtrace
 * first print to STDOUT, then to fp.
 */
static char* log_backtrace(const int max_frames, int fd){
    void *callstack[max_frames];
    int n_frames = backtrace(callstack, max_frames);

    // first output to STDOUT
    printf("backtrace() returned %d addresses\n", n_frames);
    backtrace_symbols_fd(callstack, n_frames, 1); // STDOUT

    // output to fd
    FILE* fp = NULL;
    if (fd != -1) fp = fdopen(fd, "w");
    if (fp != NULL) {
        fflush(fp);
        fprintf(fp, "backtrace() returned %d addresses\n", n_frames);

        char **symbols = backtrace_symbols(callstack, n_frames);
        for (int i = 0; i < n_frames; i++) {
            fprintf(fp, "%d: %s\n", i, symbols[i]);

            // Dl_info info;
            // char buf[1024];
            // if (dladdr(callstack[i], &info)) {
            //     snprintf(buf, sizeof(buf), "%-2d: %p\t%s\n",
            //             i, callstack[i], info.dli_sname);
            // } else {
            //     snprintf(buf, sizeof(buf), "%-2d: %p\t%s\n", i, callstack[i], symbols[i]);
            // }
            // printf(buf);
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
static jmp_buf catch_segfault; // For potential recovery


void segfault_handler(int signum) {
    LOG_FATAL("Caught segmentation fault (SIGSEGV)!!!");
    logger_flush();

    int fd = logger_filefd();
    if (fd == -1) {
        LOG_ERROR("get logger file fd failed: %d, %s", errno, strerror(errno));
    }
    log_backtrace(100, fd);

    // Perform cleanup, log information, etc.
    // It's generally not safe to continue execution after a segfault.
    // However, for specific scenarios, you might attempt recovery using longjmp.
    longjmp(catch_segfault, 1); // Jump back to a safe point
}

int set_sigsegv() {
    if (signal(SIGSEGV, segfault_handler) == SIG_ERR) {
        LOG_ERROR("Failed to register signal handler for SIGSEGV");
        return 1;
    }

    if (setjmp(catch_segfault) == 0) {
        // LOG_INFO("init catch_segfault ok.");
        // Code that might cause a segfault for test
        // int *ptr = NULL;
        // *ptr = 10; // This will cause a segfault
    } else {
        LOG_INFO("Recovered from segmentation fault (or attempted recovery).");
        // Handle the aftermath of the potential segfault
    }

    return 0;
}

