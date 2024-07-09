#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// #include "ini.h"
#include "logger.h"

#include "init_log.h"


int init_log(const char* filename, int filesize, int filenumber) {
    logger_initConsoleLogger(NULL);
    logger_initFileLogger(filename, filesize, filenumber);
    LOG_INFO("multi logging");
    logger_setLevel(LogLevel_DEBUG);
    return 0 ;
}

int ini_cb(void* arg, const char* section, const char* name, const char* value)
{
    // configuration* pconfig = (configuration*)arg;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("global", "version")) {
        printf("ini global %s, name %s, value %s\n", section, name, value);
        // pconfig->version = atoi(value);
    } else if (MATCH("user", "name")) {
        // pconfig->name = strdup(value);
    } else if (MATCH("user", "email")) {
        // pconfig->email = strdup(value);
    } else {
        printf("ini section %s, name %s, value %s\n", section, name, value);
        // return 0;  /* unknown section/name, error */
    }
    return 1;
}

int load_config(const char* filename, ini_handler ini_callback, void* arg) {
    char* tpl_filename;
    int rc;

    rc = access(filename, F_OK);
    if (0 != rc) {
        printf("ini file '%s' not exists\n", filename);
        tpl_filename = malloc(strlen(filename) + 4);
        strcpy(tpl_filename, filename);
        strcat(tpl_filename, ".tpl");
        rc = copy_file(tpl_filename, filename);
        free(tpl_filename);
        if (0 != rc) {
            printf("copy ini tpl file '%s.tpl' to ini '%s' failed\n", filename, filename);
            return -1;
        }
    }

    if (ini_parse(filename, ini_callback, arg) < 0) {
        printf("parse ini file failed '%s'\n", filename);
        return -1;
    }

    return 0;
}

int copy_file(const char* in_path, const char* out_path) {
    size_t n;
    FILE* in=NULL, * out=NULL;
    char buf[64];
    
    if((in = fopen(in_path, "rb")) && (out = fopen(out_path, "wb")))
        while((n = fread(buf, 1, sizeof(buf), in)) && fwrite(buf, 1, n, out));
    else return -1;

    if(in) fclose(in);
    if(out) fclose(out);
    return 0;
}