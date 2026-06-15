#include "init_log.h"

#include <libgen.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "logger.h"


/**
 loglevel is int, they are TRACE=0, DEBUG=1, INFO=2, WARN=3, ERROR=4, FATAL=5, default is INFO
 maxfilesize is rotate max bytes, over maxfilesize will backup file name to log.1, log.2 ...
 maxfilenumber is max filenumber to keep, eg. maxfilenumber=5, then max is log.5, erase other files.
 */
int init_log(const char* filename, int loglevel, int maxfilesize, int maxfilenumber)
{
    char tmp[4096];   // most linx PATH_MAX is 4096 bytes
    strncpy(tmp, filename, sizeof(tmp)-1);
    tmp[sizeof(tmp)-1] = '\0';

    const char* dir = dirname(tmp);
    if (-1 == access(dir, F_OK)) { // dir not existed, then mkdir
#ifdef _WIN32
        if (-1 == mkdir(dir)) {
#else
        if (-1 == mkdir(dir, 0755)) {
#endif
            printf ("mkdir log path '%s' failed, errno: %d, %s\n", dir, errno, strerror(errno));
            return -1;
        }
    }

    if (0 == logger_initConsoleLogger(NULL)) {
        printf("logger_initConsoleLogger failed\n");
    }
    if (0 == logger_initFileLogger(filename, maxfilesize, maxfilenumber)) {
        printf("logger_initFileLogger '%s' failed\n", filename);
    }

    // fulsh interval in milliseconds, 1 means always flush.
    // 100 will cause less cpu, but should call logger_flush() in timer_cb.
    logger_autoFlush(1);

    if (loglevel < LogLevel_TRACE || loglevel > LogLevel_FATAL)
        loglevel = LogLevel_INFO;
    logger_setLevel(loglevel);
    LOG_INFO ("logger_setLevel %d", loglevel);

    return 0 ;
}
