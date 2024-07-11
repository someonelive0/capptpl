#ifndef INIT_LOG_H
#define INIT_LOG_H

#include "ini.h"


int init_log(const char* filename, int debug, int filesize, int filenumber);

// config ini functions.
int ini_cb(void* arg, const char* section, const char* name, const char* value);
int load_config(const char* filename, ini_handler ini_callback, void* arg);

int copy_file(const char* in_path, const char* out_path);

#endif // INIT_LOG_H
