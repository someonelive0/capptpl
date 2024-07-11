// #include <stdlib.h>
// #include <unistd.h>
// #include <string.h>

#include "logger.h"

#include "init_log.h"


int init_log(const char* filename, int debug, int filesize, int filenumber) {
    logger_initConsoleLogger(NULL);
    logger_initFileLogger(filename, filesize, filenumber);
    if (debug) {
        logger_setLevel(LogLevel_DEBUG);
        LOG_DEBUG ("set log level to DEBUG");
    } else {
        logger_setLevel(LogLevel_INFO);
    }
    return 0 ;
}
