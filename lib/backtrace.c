#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <stddef.h>   // NULL
#include <unistd.h>   // getpid()
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

    // then output to fd if fd > 1(STDOUT)
    if (fd > 1) {
        char tmp[256] = {0};
        int rc = snprintf(tmp, sizeof(tmp)-1, "backtrace() returned %d addresses\n", n_frames);
        write(fd, tmp, rc);
        char **symbols = backtrace_symbols(callstack, n_frames);
        for (int i = 0; i < n_frames; i++) {
            Dl_info info;
            if (dladdr(callstack[i], &info) && info.dli_sname != NULL) {
                rc = snprintf(tmp, sizeof(tmp)-1, "%-2d: %p\t%s\n", i, callstack[i], info.dli_sname);
            } else {
                rc = snprintf(tmp, sizeof(tmp)-1, "%-2d: %p\t%s\n", i, callstack[i], symbols[i]);
            }
            write(fd, tmp, rc);
        }
        free(symbols);
        if (n_frames >= max_frames) {
            rc = snprintf(tmp, sizeof(tmp)-1, "[truncated]\n");
            write(fd, tmp, rc);
        }
        fsync(fd);
    }

    return NULL;
}


/*
 * param sig should be SIGSEGV
 */
static void segfault_handler(int sig, siginfo_t *info, void *ucontext) {
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

    /* dump registers, x64 CPU specific */
    #ifdef __x86_64__
    ucontext_t *context = (ucontext_t *)ucontext;
    LOG_FATAL( "Signal = %d  Memory location = %p\n"
            "RIP = %016llx  RSP = %016llx  RBP = %016llx\n"
            "RAX = %016llx  RBX = %016llx  RCX = %016llx\n"
            "RDX = %016llx  RSI = %016llx  RDI = %016llx\n"
            "R8  = %016llx  R9  = %016llx  R10 = %016llx\n"
            "R11 = %016llx  R12 = %016llx  R13 = %016llx\n"
            "R14 = %016llx  R15 = %016llx  RFLAGS = %016llx\n",
            sig, info->si_addr,
            context->uc_mcontext.gregs[REG_RIP],
            context->uc_mcontext.gregs[REG_RSP],
            context->uc_mcontext.gregs[REG_RBP],
            context->uc_mcontext.gregs[REG_RAX],
            context->uc_mcontext.gregs[REG_RBX],
            context->uc_mcontext.gregs[REG_RCX],
            context->uc_mcontext.gregs[REG_RDX],
            context->uc_mcontext.gregs[REG_RSI],
            context->uc_mcontext.gregs[REG_RDI],
            context->uc_mcontext.gregs[REG_R8],
            context->uc_mcontext.gregs[REG_R9],
            context->uc_mcontext.gregs[REG_R10],
            context->uc_mcontext.gregs[REG_R11],
            context->uc_mcontext.gregs[REG_R12],
            context->uc_mcontext.gregs[REG_R13],
            context->uc_mcontext.gregs[REG_R14],
            context->uc_mcontext.gregs[REG_R15],
            context->uc_mcontext.gregs[REG_EFL]);

    /* dump registers, aarch64 CPU specific */
    #elif defined __aarch64__
    ucontext_t *context = (ucontext_t *)ucontext;
    LOG_FATAL( "Signal = %d  Memory location = %p\n"
            "PC: 0x%llx\n"
            "SP: 0x%llx\n"
            "PSTATE: 0x%llx\n"
            "REGS = %016llx, %016llx, %016llx, %016llx\n"
            " X18 = %016llx, %016llx, %016llx, %016llx\n"
            " X29 = %016llx, %016llx\n",
            sig, info->si_addr,
            context->uc_mcontext.pc,      // Program Counter, PC
            context->uc_mcontext.sp,      // Stack Pointer, SP
            context->uc_mcontext.pstate,  // Processor State register, PSTATE
            context->uc_mcontext.regs[0], // 31 general-purpose registers (X0-X30)
            context->uc_mcontext.regs[1],
            context->uc_mcontext.regs[2],
            context->uc_mcontext.regs[3],
            context->uc_mcontext.regs[18],
            context->uc_mcontext.regs[19],
            context->uc_mcontext.regs[20],
            context->uc_mcontext.regs[21],
            context->uc_mcontext.regs[29],
            context->uc_mcontext.regs[30]);
    #endif // __aarch64__

    logger_flush();

    // now print backtrce
    int fd = logger_fileno();
    if (fd == -1) {
        LOG_ERROR("get logger file fd failed: %d, %s", errno, strerror(errno));
    }
    log_backtrace(100, fd);
    _exit(99); // when not set SA_RESETHAND, need exit() myslef. 

    // signal capture + GDB
    // char cmd[256]= {0};
    // snprintf(cmd, sizeof(cmd)-1, "sudo gdb --pid=%d -ex bt -q", getpid());
    // system(cmd);
}

int set_sigsegv_handler() {
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    // SA_RESETHAND: Restore the signal action to the default upon entry to the signal handler
    // so disable SA_RESETHAND to make enouph time to log_backtrace(), but need _exit() youself.
    // act.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
    act.sa_flags =  SA_NODEFER | SA_SIGINFO;
    act.sa_sigaction = segfault_handler;

    int rc = sigaction(SIGSEGV, &act, NULL);
    if (rc == -1) {
        return -1;
    }
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGFPE, &act, NULL);
    sigaction(SIGILL, &act, NULL);
    sigaction(SIGABRT, &act, NULL);

    return 0;
}

#else // not __linux__

int set_sigsegv_handler() { return 0; }

#endif // __linux__

