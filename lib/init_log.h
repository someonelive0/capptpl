#ifndef INIT_LOG_H
#define INIT_LOG_H


/*
 loglevel is int, they are TRACE=0, DEBUG=1, INFO=2, WARN=3, ERROR=4, FATAL=5, default is INFO
 maxfilesize is rotate max bytes, over maxfilesize will backup file name to log.1, log.2 ...
 maxfilenumber is max filenumber to keep, eg. maxfilenumber=5, then max is log.5, erase other files.
 */
int init_log(const char* filename, int loglevel, int maxfilesize, int maxfilenumber);


#endif // INIT_LOG_H
