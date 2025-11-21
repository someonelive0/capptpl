#ifndef BACKTRACE_H
#define BACKTRACE_H

int set_sigsegv_handler();
char* log_backtrace(const int max_frames, int fd);

#endif // BACKTRACE_H
