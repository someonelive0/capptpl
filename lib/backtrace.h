#ifndef BACKTRACE_H
#define BACKTRACE_H

/*
 * Usage: call set_sigsegv_handler() in main() after init_log
 */
int set_sigsegv_handler();

/*
 * print backtrace to STDOUT, also to fd when fd > 1
 */
char* log_backtrace(const int max_frames, int fd);

#endif // BACKTRACE_H
