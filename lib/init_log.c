// #include <stdlib.h>
// #include <unistd.h>
// #include <string.h>

#include "logger.h"

#include "init_log.h"


int init_log(const char* filename, int debug, int filesize, int filenumber)
{
    if (0 == logger_initConsoleLogger(NULL)) {
        printf("logger_initConsoleLogger failed\n");
    }
    if (0 == logger_initFileLogger(filename, filesize, filenumber)) {
        printf("logger_initConsoleLogger failed\n");
    }

    // fulsh interval in milliseconds, 1 means always flush.
    // 100 will cause less cpu, but should call logger_flush() in timer_cb.
    logger_autoFlush(1);

    if (debug) {
        logger_setLevel(LogLevel_DEBUG);
        LOG_DEBUG ("set log level to DEBUG");
    } else {
        logger_setLevel(LogLevel_INFO);
    }
    return 0 ;
}
