#ifndef LOAD_CONFIG_H
#define LOAD_CONFIG_H

#include "ini.h"


// ini config functions.
int ini_cb(void* arg, const char* section, const char* name, const char* value);
int load_config_ini(const char* filename, ini_handler ini_callback, void* arg);

int copy_file(const char* in_path, const char* out_path);

#endif // LOAD_CONFIG_H
